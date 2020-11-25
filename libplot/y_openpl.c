/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object, i.e., begins the drawing of
   a page of graphics.  */

/* This implementation is for XPlotters.  When invoked, it pops up a
   plotting window on the default screen of the specified X display.  When
   the corresponding closepl method is invoked, the window is `spun off',
   i.e., is managed thenceforth by a forked-off child process. */

/* This file also contains the internal functions _y_maybe_get_new_colormap
   and _y_maybe_handle_x_events.  They override the corresponding functions
   in the XDrawablePlotter superclass, which are no-ops. */

#include "sys-defines.h"
#include "extern.h"

/* song and dance to define struct timeval, and declare select() */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>		/* for struct timeval */
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		/* AIX needs this */
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* for struct fdset, FD_ZERO, FD_SET */
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* for select() */
#endif

/* fake app name, effectively our argv[0] */
#define XPLOT_APP_NAME "xplot"

/* app class, use for specifying resources */
#define XPLOT_APP_CLASS "Xplot"

/* Fallback resources for the preceding X11 class.  The default size of the
   plotting window is set here, not in y_defplot.c.  Users may override the
   default by specifying a geometry in their .Xdefaults files (by
   specifying the Xplot.geometry or xplot.geometry resource), which is
   equivalent to specifying the plotter parameter BITMAPSIZE.  There are no
   user-specifiable X resources, except for the geometry. */
static String _xplot_resources[] = 
{
  "Xplot*geometry:      570x570",
  (String)NULL 
};

/* Command-line mimicry.  This should be long enough to contain our entire
   fake argument vector, including a final NULL.  Currently, we need space
   for our fake application name, i.e. XPLOT_APP_NAME, and the "-display",
   "-geometry", "-bg" options. */
static char *_fake_argv[] = { XPLOT_APP_NAME, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
#define DEFAULT_ARGC 1		/* i.e. with no options appended yet */

/* Translations: translate any pressing of the `q' key, and any mouse
   click, to `quit if closed', i.e. quit if already forked off, and not
   otherwise. */
static const String _xplot_translations =
#ifdef USE_MOTIF
"<Btn1Down>:	Quit_if_forked()\n\
 <Btn2Down>:	ProcessDrag()\n\
 <Btn3Down>:	Quit_if_forked()\n\
 <Key>Q:	Quit_if_forked()\n\
 <Key>q:	Quit_if_forked()";
#else
"<Btn1Down>:	Quit_if_forked()\n\
 <Btn3Down>:	Quit_if_forked()\n\
 <Key>Q:	Quit_if_forked()\n\
 <Key>q:	Quit_if_forked()";
#endif

/* Data items examined when user types `q' or clicks, see Quit_if_forked()
   below. */
static bool _forked = false;
static Display *_plotter_x_dpy = NULL;
static Widget _plotter_y_toplevel = (Widget)0;

/* forward references */
static bool _bitmap_size_ok ____P((const char *bitmap_size_s));
static void Quit_if_forked ____P((Widget widget, XEvent *event, String *params, Cardinal *num_params));

#ifndef HAVE_STRERROR
static char * _plot_strerror ____P ((int errnum));
#define strerror _plot_strerror
#endif

/* This is called by the child process in y_closepl.c, after forking takes
   place.  It sets the above data items (_forked etc.), which will be
   checked whenever Quit_if_forked() is subsequently invoked, i.e. when `q'
   is pressed or a mouse click is seen. */
void
#ifdef _HAVE_PROTOS
_y_set_data_for_quitting (void)
#else
_y_set_data_for_quitting ()
#endif
{
  _forked = true;
  _plotter_x_dpy = _plotter->x_dpy;
  _plotter_y_toplevel = _plotter->y_toplevel;
}

/* Quit_if_forked() is called when `q' is pressed or a mouse button pushed.
   It does something only if closepl() has previously been invoked, so that
   _forked is true.  In that case the spun-off window disappears, and the
   forked-off process managing it exits.

   Why use the static variables _plotter_y_toplevel and _plotter_x_dpy
   here, rather than _plotter->y_toplevel and _plotter->x_dpy?  The reason
   for using them, and for the function _y_set_data_for_quitting() above
   that sets them, is that in libplotter, it would be painful, perhaps
   impossible, to make Quit_if_forked a function member of the Plotter
   class.  But it's easy enough to make _y_set_data_for_quitting() such a
   member. */

static void			
#ifdef _HAVE_PROTOS
Quit_if_forked (Widget widget, XEvent *event, String *params, Cardinal *num_params)
#else
Quit_if_forked (widget, event, params, num_params) /* an action */
     Widget widget;		
     XEvent *event;
     String *params;
     Cardinal *num_params;
#endif
{
  if (_forked)
    /* we must be a forked-off child process managing a window, so tear
       down and exit */
    {
      XtDestroyWidget (_plotter_y_toplevel);
      XFlush (_plotter_x_dpy);	/* flush output buffer before exiting */
      exit (EXIT_SUCCESS);
    }
}

/* application context-specific action table */
static XtActionsRec _xplot_actions[] = 
{
  {"Quit_if_forked",	Quit_if_forked},
};

int
#ifdef _HAVE_PROTOS
_y_openpl (void)
#else
_y_openpl ()
#endif
{
  Arg wargs[10];		/* werewolves */
  Dimension window_height, window_width;
  Screen *screen_struct;	/* screen structure */
  String *copied_fake_argv;
  Visual *visual;
  const char *bg_color_name_s, *double_buffer_s;
  int i, fake_argc, copied_fake_argc;
  int screen;			/* screen number */
  static bool toolkit_initted = false;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* To permit openpl..closepl to be invoked repeatedly, we don't use the
     convenience routine XtAppInitialize(), since that function starts out
     by calling XtToolkitInitialize(), which shouldn't be called more than
     once.  Instead, we call XtToolkitInitialize on the first invocation of
     openpl().  On every invocation of openpl(), including the first, we
     call the other four functions that XtAppInitialize would call:
     XtCreateApplicationContext, XtAppSetFallbackResources, XtOpenDisplay,
     and XtAppCreateShell.  That sets up a new application context each
     time openpl() is called, which looks wasteful.  But since each
     openpl..closepl will yield a window managed by a forked-off process,
     it's appropriate. */

  if (!_plotter->opened)
    {
      if (!toolkit_initted)
	/* no previous X Plotter has been created and opened */
	{
	  XtToolkitInitialize();
	  toolkit_initted = true;
	}
    }

  /* create new application context for this Plotter page */
  _plotter->y_app_con = XtCreateApplicationContext();
  if (_plotter->y_app_con == (XtAppContext)NULL)
    {
      _plotter->error ("can't create X application context");
      return -2;
    }
  /* set fallback resources (currently, only the window size) to be used by
     canvas widget; specific to application context */
  XtAppSetFallbackResources (_plotter->y_app_con, _xplot_resources);

  /* register an action table [currently containing only
     "Quit_if_forked"->Quit_if_forked(), see above]; specific to
     application context */
  XtAppAddActions (_plotter->y_app_con, _xplot_actions,
		   XtNumber (_xplot_actions));
  
  /* punch options and parameters into the fake command-line option list */
  fake_argc = DEFAULT_ARGC;

  /* take argument of the "-display" option from the DISPLAY parameter */
  {
    const char *display_s;
    char *copied_display_s;
	
    display_s = (const char *)_get_plot_param ("DISPLAY");
    if (display_s == NULL || *display_s == '\0')
      {
	_plotter->error ("can't open Plotter, DISPLAY parameter is null");
	return -1;
      }
    copied_display_s = (char *)_plot_xmalloc(strlen (display_s) + 1);
    strcpy (copied_display_s, display_s);
    _fake_argv[fake_argc++] = (String)"-display";
    _fake_argv[fake_argc++] = (String)copied_display_s;
  }
  
  /* Take argument of "-geometry" option from BITMAPSIZE parameter, if set;
     otherwise size will be taken from Xplot.geometry.  Fallback size is
     specified at head of this file. */
  {
    const char *bitmap_size_s;
    char *copied_bitmap_size_s;
	
    bitmap_size_s = (const char *)_get_plot_param ("BITMAPSIZE");
    if (bitmap_size_s && _bitmap_size_ok (bitmap_size_s))
      {
	copied_bitmap_size_s = (char *)_plot_xmalloc(strlen (bitmap_size_s) + 1);
	strcpy (copied_bitmap_size_s, bitmap_size_s);
	_fake_argv[fake_argc++] = (String)"-geometry";
	_fake_argv[fake_argc++] = (String)copied_bitmap_size_s;
      }
  }

  /* Take argument of "-bg" option from BG_COLOR parameter, if set.  Could
     be improved because use of widget "-bg" option means that we recognize
     only the color names known to the X server, not the slightly expanded
     set in g_colorname.h.  FIXME. */
  {
    const char *bg_color_s;
    char *copied_bg_color_s;
	
    bg_color_s = (const char *)_get_plot_param ("BG_COLOR");
    if (bg_color_s)
      {
	const Colornameinfo *info;

	if (_plotter->bg_color_warning_issued == false 
	    && _string_to_color (bg_color_s, &info) == false)
	  {

	    char *buf;
		
	    buf = (char *)_plot_xmalloc (strlen (bg_color_s) + 100);
	    sprintf (buf, "substituting \"white\" for undefined background color \"%s\"", 
		     bg_color_s);
	    _plotter->warning (buf);
	    free (buf);
	    _plotter->bg_color_warning_issued = true;

	    bg_color_s = "white";
	  }
	copied_bg_color_s = (char *)_plot_xmalloc(strlen (bg_color_s) + 1);
	strcpy (copied_bg_color_s, bg_color_s);
	_fake_argv[fake_argc++] = (String)"-bg";
	_fake_argv[fake_argc++] = (String)copied_bg_color_s;
      }
  }

  /* copy fake command-line option list */
  copied_fake_argv = (String *)_plot_xmalloc((fake_argc + 1) * sizeof(String));
  for (i = 0 ; i < fake_argc ; i++) 
    copied_fake_argv[i] = (String)(_fake_argv[i]);
  copied_fake_argv[i] = (String)NULL; /* append final NULL */
  copied_fake_argc = fake_argc;
  
  /* open new connection to the X display, using fake argv */
  _plotter->x_dpy = 
    XtOpenDisplay (_plotter->y_app_con,
		   /* display_string = NULL, so take from fake commandline */
		   (String)NULL, 
		   /* application name = NULL, so take from fake commandline */
		   (String)NULL,	
		   /* application class */
		   (String)XPLOT_APP_CLASS, 
		   /* application-specific commandline parsetable (for
		      XrmParseCommand), used in setting display resources */
		   NULL, (Cardinal)0, 
		   /* pass fake command-line (contains a fake argv[0] to
		      specify app name, and besides "-display", options may
		      include "-geometry", "-bg") */
		   &copied_fake_argc, copied_fake_argv);
  if (_plotter->x_dpy == (Display *)NULL)
    {
      char *display_s;

      display_s = (char *)_get_plot_param ("DISPLAY");
      if (display_s == NULL)	/* shouldn't happen */
	_plotter->error ("can't open null X Window System display");
      else
	{
	  char *buf;

	  buf = (char *)_plot_xmalloc(strlen(display_s) + 1 + 50);
	  sprintf (buf, "can't open X Window System display \"%s\"", 
		   display_s);
	  _plotter->error (buf);
	  free (buf);
	}
      return -2;
    }
  
  /* display was opened, so determine its default screen, visual, colormap */
  screen = DefaultScreen (_plotter->x_dpy);
  screen_struct = ScreenOfDisplay (_plotter->x_dpy, screen);
  visual = DefaultVisualOfScreen (screen_struct);
  _plotter->x_cmap = DefaultColormapOfScreen (screen_struct);
  _plotter->x_cmap_type = CMAP_ORIG; /* original cmap (not a private one) */
  
  /* find out how long polylines can get */
  _plotter->hard_polyline_length_limit = 
    XMaxRequestSize(_plotter->x_dpy) / 2;
  
  /* For every invocation of openpl(), we create a toplevel Shell widget,
     associated with default screen of the opened display.  (N.B. could
     vary name of app instance; also select a non-default colormap by
     setting a value for XtNcolormap.) */
  XtSetArg(wargs[0], XtNscreen, screen_struct);
  XtSetArg(wargs[1], XtNargc, copied_fake_argc);
  XtSetArg(wargs[2], XtNargv, copied_fake_argv);
  _plotter->y_toplevel = XtAppCreateShell(NULL, /* name of app instance */
			     (String)XPLOT_APP_CLASS, /* app class */
			     applicationShellWidgetClass, 
			     _plotter->x_dpy, /* x_dpy to get resources from */
			     /* pass XtNscreen resource, and also fake
				command-line, to get resources from
				(options may include "-display"
				[redundant], and "-geometry", "-bg") */
			     wargs, (Cardinal)3); 
  free (copied_fake_argv);

  /* Create drawing canvas (a Label widget) as child of toplevel Shell
     widget.  Set many obscure spacing parameters to zero, so that origin
     of bitmap will coincide with upper left corner of window. */
#ifdef USE_MOTIF
  XtSetArg(wargs[0], XmNmarginHeight, (Dimension)0);
  XtSetArg(wargs[1], XmNmarginWidth, (Dimension)0);
  XtSetArg(wargs[2], XmNmarginLeft, (Dimension)0);
  XtSetArg(wargs[3], XmNmarginRight, (Dimension)0);
  XtSetArg(wargs[4], XmNmarginTop, (Dimension)0);
  XtSetArg(wargs[5], XmNmarginBottom, (Dimension)0);
  XtSetArg(wargs[6], XmNshadowThickness, (Dimension)0);
  XtSetArg(wargs[7], XmNhighlightThickness, (Dimension)0);
  _plotter->y_canvas = XtCreateManagedWidget ((String)"", xmLabelWidgetClass,
					    _plotter->y_toplevel, 
					    wargs, (Cardinal)8);
#else  
  XtSetArg(wargs[0], XtNinternalHeight, (Dimension)0);
  XtSetArg(wargs[1], XtNinternalWidth, (Dimension)0);
  _plotter->y_canvas = XtCreateManagedWidget ((String)"", labelWidgetClass,
					    _plotter->y_toplevel, 
					    wargs, (Cardinal)2);
#endif
  
  /* realize both widgets */
  XtRealizeWidget (_plotter->y_toplevel);
  
  /* replace the Label widget's default translations by ours [see above;
     mostly just `q'->Quit_if_forked, with a nod to Motif] */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNtranslations, 
	    XtParseTranslationTable(_xplot_translations));
#else
  XtSetArg (wargs[0], XtNtranslations, 
	    XtParseTranslationTable(_xplot_translations));
#endif
  XtSetValues (_plotter->y_canvas, wargs, (Cardinal)1);

  /* get Label widget's window; store it in Plotter struct as
     `drawable #2' */
  _plotter->x_drawable2 = (Drawable)XtWindow(_plotter->y_canvas);

  /* get the window size that was actually chosen, store it */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNwidth, &window_width);
  XtSetArg (wargs[1], XmNheight, &window_height);
#else
  XtSetArg (wargs[0], XtNwidth, &window_width);
  XtSetArg (wargs[1], XtNheight, &window_height);
#endif
  XtGetValues (_plotter->y_canvas, wargs, (Cardinal)2);
  _plotter->imin = 0;
  _plotter->imax = (int)window_width - 1;
  /* note flipped-y convention for this device: min > max */
  _plotter->jmin = (int)window_height - 1;
  _plotter->jmax = 0;

  /* request backing store for Label widget's window */
  if (DoesBackingStore(screen_struct))
    {
      XSetWindowAttributes attributes;
      unsigned long value_mask;

      attributes.backing_store = Always;
      value_mask = CWBackingStore;
      XChangeWindowAttributes (_plotter->x_dpy, (Window)_plotter->x_drawable2, 
			       value_mask, &attributes);
    }

  /* determine whether to use double buffering */
  _plotter->x_double_buffering = DBL_NONE;
  double_buffer_s = (const char *)_get_plot_param ("USE_DOUBLE_BUFFERING");

  /* we don't distinguish "fast" from "yes" any more */
  if (strcmp (double_buffer_s, "yes") == 0)
    double_buffer_s = "fast";

#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#ifdef HAVE_DBE_SUPPORT
  if (strcmp (double_buffer_s, "fast") == 0)
    /* check whether X server supports DBE extension */
    {
      int major_version, minor_version;
      int one = 1;
      XdbeScreenVisualInfo *sv_info;
      
      if (XdbeQueryExtension (_plotter->x_dpy, &major_version, &minor_version)
	  && (sv_info = XdbeGetVisualInfo (_plotter->x_dpy, 
					   /* 2nd arg specifies screen */
					   &_plotter->x_drawable2, 
					   &one)) != NULL)
	/* server supports DBE extension */
	{
	  bool ok = false;
	  int i, num_visuals = sv_info->count;
	  XdbeVisualInfo *vis_info = sv_info->visinfo;
	  VisualID visual_id = XVisualIDFromVisual (visual);

	  /* See whether default visual supports double buffering.  Could
	     also check the depth and perflevel of each visual that
	     supports double buffering, select the best one, and call
	     XCreateColormap() to create a colormap of that visual type.
	     Maybe someday.  */
	  for (i = 0; i < num_visuals; i++)
	    if (vis_info[i].visual == visual_id) /* `visual_id' is default */
	      {
		ok = true;
		break;
	      }
	  XdbeFreeVisualInfo (sv_info);
	  if (ok)
	    /* allocate back buffer, to serve as our graphics buffer;
	       save it as `x_drawable3' */
	    {
	      _plotter->x_drawable3 = 
		XdbeAllocateBackBufferName (_plotter->x_dpy,
					    _plotter->x_drawable2, 
					    XdbeUndefined);
	      /* set double buffering type in Plotter structure */
	      _plotter->x_double_buffering = DBL_DBE;
	    }
	}
    }
#endif /* HAVE_DBE_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_XDBE_H */

#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#ifdef HAVE_MBX_SUPPORT
  if (_plotter->x_double_buffering == DBL_NONE
      && strcmp (double_buffer_s, "fast") == 0)
    /* check whether X server supports MBX extension */
    {
      int event_base, error_base;
      int major_version, minor_version;
      
      if (XmbufQueryExtension (_plotter->x_dpy, &event_base, &error_base)
	  && XmbufGetVersion (_plotter->x_dpy, &major_version, &minor_version))
	/* server supports MBX extension */
	{
	  Multibuffer multibuf[2];
	  int num;
	  
	  num = XmbufCreateBuffers (_plotter->x_dpy, 
				    (Window)_plotter->x_drawable2, 2, 
				    MultibufferUpdateActionUndefined,
				    MultibufferUpdateHintFrequent,
				    multibuf);
	  if (num == 2)
	    /* Yow, got a pair of multibuffers.  We'll write graphics to
	       the first (`x_drawable3'), and interchange them on each
	       erase().  See x_erase.c. */
	    {
	      _plotter->x_drawable3 = multibuf[0];
	      _plotter->y_drawable4 = multibuf[1];	      
	      /* set double buffering type in Plotter structure */
	      _plotter->x_double_buffering = DBL_MBX;
	    }
	  else
	    _plotter->warning ("X server refuses to support multibuffering");
	}
    }
#endif /* HAVE_MBX_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_MULTIBUF_H */

  if (_plotter->x_double_buffering == DBL_NONE)
    /* user didn't request server-supported double buffering, or did but it
       couldn't be obtained */
    {
      Pixmap bg_pixmap;

      /* create background pixmap for Label widget; 2nd arg (window) is
         only used for determining the screen */
      bg_pixmap = XCreatePixmap(_plotter->x_dpy, 
				_plotter->x_drawable2,
				(unsigned int)window_width, 
				(unsigned int)window_height, 
				(unsigned int)PlanesOfScreen(screen_struct));
      /* If user requested normal double buffering ("yes" rather than
	 "fast"), we'll double buffer `by hand': we'll use this pixmap as
	 the only drawable into which we'll draw.  If user didn't request
	 double buffering, use it as the 2nd of two drawables into which
	 we'll draw, the other being the window. */
      if (strcmp (double_buffer_s, "yes") == 0
	  || strcmp (double_buffer_s, "fast") == 0)
	{
	  _plotter->x_drawable3 = (Drawable)bg_pixmap;
	  _plotter->x_double_buffering = DBL_BY_HAND;
	}
      else
	{
	  _plotter->x_drawable1 = (Drawable)bg_pixmap;
	  _plotter->x_double_buffering = DBL_NONE;
	}
    }

  /* Invoke generic method; among other things, this invokes savestate() to
     create a drawing state (see x_savestate.c).  The drawing state won't
     be ready for drawing graphics, since it won't contain an X font or a
     meaningful line width.  To retrieve an X font and set the line width,
     the user will need to invoke space() after openpl(). */
  _g_openpl ();

  /* set background color (possibly user-specified) in drawing state */
  bg_color_name_s = (const char *)_get_plot_param ("BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (bg_color_name_s);
  
  /* If not double-buffering, clear both pixmap and window by filling them
     with the background color, via XFillRectangle (the just-created
     drawing state, with the mentioned attributes, is used for this).
     If double buffering, do something similar (see the code). */
  _plotter->erase ();
  
  /* If double buffering, must invoke `erase' one more time to clear both
     graphics buffer and window, since what `erase' does in that case is
     (1) copy the graphics buffer to window, and (2) clear the graphics
     buffer.  Incidentally, invoking `erase' bumps _plotter->frame_number,
     but that's not too important. */
  if (_plotter->x_double_buffering != DBL_NONE) 
    _plotter->erase ();

  if (_plotter->x_double_buffering == DBL_NONE
      || _plotter->x_double_buffering == DBL_BY_HAND)
    /* have a pixmap, so install it as Label widget's background pixmap */
    {
      Pixmap bg_pixmap;
      
      bg_pixmap = ((_plotter->x_double_buffering == DBL_BY_HAND) ? 
		   _plotter->x_drawable3 : _plotter->x_drawable1);
#ifdef USE_MOTIF
      XtSetArg (wargs[0], XmNlabelPixmap, bg_pixmap);
      XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
      XtSetValues (_plotter->y_canvas, wargs, (Cardinal)2);
#else
      XtSetArg (wargs[0], XtNbitmap, bg_pixmap);
      XtSetValues (_plotter->y_canvas, wargs, (Cardinal)1);
#endif
    }

  /* do an XSync on the display (this will cause the background color to
   show up if it hasn't already) */
  _plotter->flushpl ();

  return 0;
}

static bool 
#ifdef _HAVE_PROTOS
_bitmap_size_ok (const char *bitmap_size_s)
#else
_bitmap_size_ok (bitmap_size_s)
     const char *bitmap_size_s;
#endif
{
  int width, height;
  
  if (bitmap_size_s
      /* should parse this better */
      && (sscanf (bitmap_size_s, "%dx%d", &width, &height) == 2)
      && (width > 0) && (height > 0))
    return true;
  else
    return false;
}

/* This is the XPlotter-specific version of the _maybe_get_new_colormap()
   method, which is invoked when a Plotter's original colormap fills up.
   It overrides the XDrawable-specific version, which is a no-op. */
void
#ifdef _HAVE_PROTOS
_y_maybe_get_new_colormap (void)
#else
_y_maybe_get_new_colormap ()
#endif
{
  Colormap new_x_cmap;
  
  /* sanity check */
  if (_plotter->x_cmap_type != CMAP_ORIG)
    return;

  _plotter->warning ("color supply low, switching to private colormap");
  new_x_cmap = XCopyColormapAndFree (_plotter->x_dpy, _plotter->x_cmap);

  if (new_x_cmap == 0)
    /* couldn't create colormap */
    {
      _plotter->warning ("unable to create private colormap");
      _plotter->warning ("color supply exhausted, can't create new colors");
      _plotter->x_color_warning_issued = true;
    }
  else
    /* got a new colormap */
    {
      Arg wargs[10];		/* werewolves */

      /* place in Plotter, flag as new */
      _plotter->x_cmap = new_x_cmap;
      _plotter->x_cmap_type = CMAP_NEW;

      /* switch to it: install in y_toplevel shell widget */
      XtSetArg (wargs[0], XtNcolormap, _plotter->x_cmap);
      XtSetValues (_plotter->y_toplevel, wargs, (Cardinal)1);
    }
  
  return;
}

/* This is the XPlotter-specific version of the _maybe_handle_x_events()
   method, which is invoked at the end of most drawing operations.  It
   overrides the XDrawablePlotter-specific version, which is a no-op.  It
   does two things.

   1. It invokes XFlush() to flush the X output buffer.  This makes most
      drawing operations more or less unbuffered: as the libplot functions
      are invoked, the graphics are sent to the X display.

   2. It scans through the _xplotters[] sparse array, which contains
      pointers to all currently existing XPlotters, and processes pending
      X events associated with any of their application contexts.

   Why do we do #2?  Once closepl() has been invoked on an XPlotter, the
   window popped up by openpl() is managed by a forked-off process via
   XtAppMainLoop().  But until that time, we must process events manually.
   (To speed up drawing, we perform #2 only once per
   X_EVENT_HANDLING_PERIOD invocations of this function.)

   #2 is accomplished by the local event loop:

       while (XtAppPending (_xplotters[i]->y_app_con))
         XtAppProcessEvent (_xplotters[i]->y_app_con, XtIMAll);

   which, for Plotter number i, flushes the X output buffer, checks for
   events and processes them.  

   Nathan Salwen <salwen@physics.harvard.edu> has discovered that before
   invoking XtAppPending(), we must manually check, using Xlib calls and
   select(), whether there are events waiting (either in the X input
   buffer, or on the network socket).  The reason is that if no events are
   available, XtAppPending() may block, at least on some systems.  This
   does not agree with Xt documentation, but happens nonetheless.  And it
   is not what we want.

   The reason for XtAppPending's unfortunate behavior is apparently the
   following: it invokes the Xlib function XEventsQueued(), first with
   mode=QueuedAfterReading [which returns the number of events in the input
   queue if nonempty; if empty, tries to extract more events from the
   socket] and then with mode=QueuedAfterFlush [which flushes the output
   buffer with XFlush() and checks if there is anything in the input queue;
   if not, it tries to extract more events from the socket].  (N.B. If,
   alternatively, it used mode=QueuedAlready, it would look only at the
   number of events in the input queue.)  And when trying to extract events
   from the socket, XEventsQueued() calls select() in such a way that it
   can block.

   So before invoking XtAppPending() we call select() ourselves to check
   whether data is available, and we don't allow it to block. */

#define X_EVENT_HANDLING_PERIOD 4

void
#ifdef _HAVE_PROTOS
_y_maybe_handle_x_events(void)
#else
_y_maybe_handle_x_events()
#endif
{
  static int count = 0;

  /* Flush output buffer if we're *not* in the middle of constructing a
     path, or if we are, but the path will be drawn with a solid,
     zero-width pen.  Latter is for consistency with our convention that
     solid, zero-width paths should appear on the display as they're drawn
     (see x_cont.c). */
  if (_plotter->drawstate->points_in_path == 0
      || (_plotter->drawstate->line_type == L_SOLID
	  && !_plotter->drawstate->dash_array_in_effect
	  && _plotter->drawstate->points_are_connected
	  && _plotter->drawstate->quantized_device_line_width == 0))
    XFlush (_plotter->x_dpy);

  if (count % X_EVENT_HANDLING_PERIOD == 0)
    /* process events, if any are available */
    {
      int i;
      bool have_data;

      for (i = 0; i < _xplotters_len; i++)
	{
	  have_data = false;	/* default */

	  if (_xplotters[i] != NULL
	      && _xplotters[i]->opened
	      && _xplotters[i]->open
	      && _xplotters[i]->y_app_con != NULL) /* paranoia */
	    {
	      if (QLength(_xplotters[i]->x_dpy) > 0)
		/* one or more events has already been pulled off the
		   socket and is in the input queue, so we'll invoke
		   XtAppPending() */
		have_data = true;
	    
	      else
		/* input queue is empty, so check whether data is available
                   on the socket itself */
		{
		  int connection_number;
		  int maxfds, select_return;
		  fd_set readfds;
		  struct timeval timeout;
		  
		  timeout.tv_sec = 0;	/* make select() non-blocking! */
		  timeout.tv_usec = 0;
		  
		  connection_number = ConnectionNumber(_xplotters[i]->x_dpy);
		  maxfds = 1 + connection_number;
		  FD_ZERO (&readfds);
		  FD_SET (connection_number, &readfds);
		  select_return = 
		    select (maxfds, &readfds, NULL, NULL, &timeout);
		  
		  if (select_return < 0 && errno != EINTR)
		    {
		      _plotter->error (strerror (errno));
		      continue;	/* on to next Plotter */
		    }
		  if (select_return > 0)
		    /* have data waiting on the socket, waiting to be
		       pulled off, so we'll invoke XtAppPending() */
		    have_data = true;
		}

	      if (have_data)
		/* Call XtAppPending() to read event(s) from queue, if
		   nonempty, or from the socket itself.  In the latter
		   case, XtAppPending() will call XEventsQueued(), which
		   will, in turn, call select().  One way or the other, we
		   should get at least one event, without blocking. */
		{
		  while (XtAppPending (_xplotters[i]->y_app_con))
		    XtAppProcessEvent (_xplotters[i]->y_app_con, XtIMAll);
		}
	    }
	}
    }
  count++;
}

#ifndef HAVE_STRERROR
/* A libplot-specific version of strerror(), for very old systems that
   don't have one. */

extern char *sys_errlist[];
extern int sys_nerr;

static char *
#ifdef _HAVE_PROTOS
_plot_strerror (int errnum)
#else
_plot_strerror (errnum)
     int errnum;
#endif
{
  if (errnum < 0 || errnum >= sys_nerr)
    return "unknown error";

  return sys_errlist[errnum];
}
#endif
