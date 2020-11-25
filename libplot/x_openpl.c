/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object, i.e., begins the drawing of
   a page of graphics. 

   This implementation is for XPlotters.  When invoked, it pops up a
   plotting window on the default screen of the specified X display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define XPLOT_APP_NAME "xplot"
#define XPLOT_APP_CLASS "Xplot"

/* Fallback resources for the preceding class.  The default size of the
   plotting window is set here, not in extern.h.  Users may override the
   default by specifying a geometry manually, in their .Xdefaults files (by
   specifying the Xplot.geometry or xplot.geometry resource), and of course
   by specifying the plotter parameter BITMAPSIZE.

   The foreground and background color resources are ignored.  We
   initialize foreground and background respectively to black and to white
   (or to whatever the user has set programmatically as the value of the
   BG_COLOR plotter parameter). */
static String _xplot_resources[] = 
{
  "Xplot*geometry:      570x570",
#ifdef USE_MOTIF
  "Xplot*shadowThickness:	0",
  "Xplot*highlightThickness:	0",
  "Xplot*marginHeight:  0",
  "Xplot*marginWidth:   0",
#else
  "Xplot*internalHeight: 0",
  "Xplot*internalWidth: 0",
#endif
  (String)NULL 
};

/* Command-line mimicry: we'll punch in the X display, and possibly
   geometry and background color also.  This should be long enough to
   contain the entire argument vector, including a final NULL. */
static char *_fake_argv[] = { XPLOT_APP_NAME, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
#define DEFAULT_ARGC 1		/* i.e. with no options appended */

/* translate mouse clicks to `quit if closed', i.e. quit if already forked
   off, and not otherwise */
static const String _xplot_translations =
#ifdef USE_MOTIF
"<Btn1Down>:	quit_if_closed()\n\
 <Btn2Down>:	ProcessDrag()\n\
 <Btn3Down>:	quit_if_closed()\n\
 <Key>Q:	quit_if_closed()\n\
 <Key>q:	quit_if_closed()";
#else
"<Btn1Down>:	quit_if_closed()\n\
 <Btn3Down>:	quit_if_closed()\n\
 <Key>Q:	quit_if_closed()\n\
 <Key>q:	quit_if_closed()";
#endif

/* forward references */
static void Quit_if_closed __P((Widget widget, XEvent *event, String *params, Cardinal *num_params));
static bool _bitmap_size_ok __P((const char *bitmap_size_s));

/* Quit_if_closed() is called when `q' is pressed or a mouse button pushed,
   to close a spun-off window (and kill the forked-off process managing it) */
static void			
#ifdef _HAVE_PROTOS
Quit_if_closed (Widget widget, XEvent *event, String *params, Cardinal *num_params)
#else
Quit_if_closed (widget, event, params, num_params) /* an action */
     Widget widget;		
     XEvent *event;
     String *params;
     Cardinal *num_params;
#endif
{
  if (_plotter->opened && !_plotter->open)
    /* we must be a forked-off child process managing a closed window, so
       tear down and exit */
    {
      XtDestroyWidget (_plotter->toplevel);
      XFlush (_plotter->dpy);	/* flush output buffer before exiting */
      exit (EXIT_SUCCESS);
    }
}

/* application context-specific action table */
static XtActionsRec _xplot_actions[] = 
{
  {"quit_if_closed",	Quit_if_closed},
};

int
#ifdef _HAVE_PROTOS
_x_openpl (void)
#else
_x_openpl ()
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
  _plotter->app_con = XtCreateApplicationContext();
  if (_plotter->app_con == (XtAppContext)NULL)
    {
      _plotter->error ("can't create X application context");
      return -2;
    }
  /* set misc. resources (including fallback window size) to be used by
     canvas widget; specific to application context */
  XtAppSetFallbackResources (_plotter->app_con, _xplot_resources);

  /* register an action table [currently containing only
     "quit_if_closed->Quit_if_closed", see above]; 
     specific to application context */
  XtAppAddActions (_plotter->app_con, _xplot_actions,
		   XtNumber (_xplot_actions));
  
  /* punch options and parameters into the fake command-line option list */
  fake_argc = DEFAULT_ARGC;

  /* take argument of the "-display" option from the DISPLAY parameter */
  {
    const char *display_s;
    char *copied_display_s;
	
    display_s = (const char *)_get_plot_param (_plotter, "DISPLAY");
    if (display_s == NULL)
      {
	_plotter->error ("can't open null X Window System display");
	return -1;
      }
    copied_display_s = (char *)_plot_xmalloc(strlen (display_s) + 1);
    strcpy (copied_display_s, display_s);
    _fake_argv[fake_argc++] = (String)"-display";
    _fake_argv[fake_argc++] = (String)copied_display_s;
  }
  
  /* take argument of "-geometry" option from BITMAPSIZE parameter, if set */
  {
    const char *bitmap_size_s;
    char *copied_bitmap_size_s;
	
    bitmap_size_s = (const char *)_get_plot_param (_plotter, "BITMAPSIZE");
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
	
    bg_color_s = (const char *)_get_plot_param (_plotter, "BG_COLOR");
    if (bg_color_s)
      {
	const Colornameinfo *info;

	if (_plotter->bg_color_warning_issued == false 
	    && _string_to_color (bg_color_s, &info) == false)
	  {
	    if (true)
	      {
		char *buf;
		
		buf = (char *)_plot_xmalloc (strlen (bg_color_s) + 100);
		sprintf (buf, "substituting \"white\" for undefined background color \"%s\"", 
			 bg_color_s);
		_plotter->warning (buf);
		free (buf);
		_plotter->bg_color_warning_issued = true;
	      }
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
  
  /* open new connection to the X display, using fake option list */
  _plotter->dpy = 
    XtOpenDisplay (_plotter->app_con,
		   /* display_string = NULL, so take from fake commandline */
		   (String)NULL, 
		   /* application name = NULL, so take from fake commandline */
		   (String)NULL,	
		   /* application class */
		   (String)XPLOT_APP_CLASS, 
		   /* application-specific commandline parsetable (for
		      XrmParseCommand), used in setting display resources */
		   NULL, (Cardinal)0, 
		   /* fake commandline */
		   &copied_fake_argc, copied_fake_argv);
  if (_plotter->dpy == (Display *)NULL)
    {
      char *display_s;

      display_s = (char *)_get_plot_param (_plotter, "DISPLAY");
      if (display_s == NULL)	/* shouldn't happen */
	_plotter->error ("can't open null X Window System display");
      else
	{
	  int len;
	  char *buf;

	  len = strlen (display_s);
	  buf = (char *)_plot_xmalloc(strlen(display_s) + 1 + 50);
	  sprintf (buf, "can't open X Window System display \"%s\"", 
		   display_s);
	  _plotter->error (buf);
	  free (buf);
	}
      return -2;
    }
  
  /* display was opened, so determine its default screen, visual, colormap */
  screen = DefaultScreen (_plotter->dpy);
  screen_struct = ScreenOfDisplay (_plotter->dpy, screen);
  visual = DefaultVisualOfScreen (screen_struct);
  _plotter->cmap = DefaultColormapOfScreen (screen_struct);
  
  /* find out how long polylines can get */
  _plotter->hard_polyline_length_limit = 
    XMaxRequestSize(_plotter->dpy) / 2;
  
  /* For every invocation of openpl(), we create a toplevel widget,
     associated with default screen of the opened display.  (N.B. could
     vary name of app instance from page to page; also select a non-default
     colormap by setting a value for XtNcolormap.) */
  XtSetArg(wargs[0], XtNscreen, screen_struct);
  XtSetArg(wargs[1], XtNargc, copied_fake_argc);
  XtSetArg(wargs[2], XtNargv, copied_fake_argv);
  _plotter->toplevel = XtAppCreateShell(NULL, /* name of app instance */
			     (String)XPLOT_APP_CLASS, /* app class */
			     applicationShellWidgetClass, 
			     _plotter->dpy, /* dpy to get resources from */
			     /* fake command-line option list,
				to get additional resources from */
			     wargs, (Cardinal)3); 
  free (copied_fake_argv);

  /* Create drawing canvas (a Label widget) as child of toplevel widget.
     If user set the BITMAPSIZE parameter it determines the size; otherwise
     size taken from xplot.geometry.  Fallback size is specified at head of
     this file. */
#ifdef USE_MOTIF
  _plotter->canvas = XtCreateManagedWidget ((String)"", xmLabelWidgetClass,
					    _plotter->toplevel, 
					    NULL, (Cardinal)0);
#else  
  _plotter->canvas = XtCreateManagedWidget ((String)"", labelWidgetClass,
					    _plotter->toplevel, 
					    NULL, (Cardinal)0);
#endif
  
  /* realize both widgets */
  XtRealizeWidget (_plotter->toplevel);
  
  /* replace the Label widget's default translations by ours [see above;
     mostly just `q'->quit_if_closed, with a nod to Motif] */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNtranslations, 
	    XtParseTranslationTable(_xplot_translations));
#else
  XtSetArg (wargs[0], XtNtranslations, 
	    XtParseTranslationTable(_xplot_translations));
#endif
  XtSetValues (_plotter->canvas, wargs, (Cardinal)1);

  /* get Label widget's window; store it in Plotter struct as
     `drawable #2' */
  _plotter->drawable2 = (Drawable)XtWindow(_plotter->canvas);

  /* get the window size that was actually chosen, store it */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNwidth, &window_width);
  XtSetArg (wargs[1], XmNheight, &window_height);
#else
  XtSetArg (wargs[0], XtNwidth, &window_width);
  XtSetArg (wargs[1], XtNheight, &window_height);
#endif
  XtGetValues (_plotter->canvas, wargs, (Cardinal)2);
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
      XChangeWindowAttributes (_plotter->dpy, (Window)_plotter->drawable2, 
			       value_mask, &attributes);
    }

  /* determine type of double buffering to use (if any) */
  _plotter->double_buffering = DBL_NONE;
  double_buffer_s = (const char *)_get_plot_param (_plotter, "USE_DOUBLE_BUFFERING");

#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#ifdef HAVE_DBE_SUPPORT
  if (strcmp (double_buffer_s, "fast") == 0)
    /* check whether X server supports DBE extension */
    {
      int major_version, minor_version;
      int one = 1;
      XdbeScreenVisualInfo *sv_info;
      
      if (XdbeQueryExtension (_plotter->dpy, &major_version, &minor_version)
	  && (sv_info = XdbeGetVisualInfo (_plotter->dpy, 
					   /* 2nd arg specifies screen */
				   &_plotter->drawable2, &one)) != NULL)
	/* server supports DBE extension */
	{
	  bool ok = false;
	  int i, num_visuals = sv_info->count;
	  XdbeVisualInfo *vis_info = sv_info->visinfo;
	  VisualID visual_id = XVisualIDFromVisual (visual);

	  /* See whether default visual supports double buffering.  Could
	     also check the depth and perflevel of each visual that
	     supports double buffering, select the best one, and call
	     XCreateColormap() to create a cmap of that visual type.
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
	       save it as `drawable3' */
	    {
	      _plotter->drawable3 = 
		XdbeAllocateBackBufferName (_plotter->dpy,
					    _plotter->drawable2, 
					    XdbeUndefined);
	      /* set double buffering type in Plotter structure */
	      _plotter->double_buffering = DBL_DBE;
	    }
	}
    }
#endif /* HAVE_DBE_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_XDBE_H */

#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#ifdef HAVE_MBX_SUPPORT
  if (_plotter->double_buffering == DBL_NONE
      && strcmp (double_buffer_s, "fast") == 0)
    /* check whether X server supports MBX extension */
    {
      int event_base, error_base;
      int major_version, minor_version;
      
      if (XmbufQueryExtension (_plotter->dpy, &event_base, &error_base)
	  && XmbufGetVersion (_plotter->dpy, &major_version, &minor_version))
	/* server supports MBX extension */
	{
	  Multibuffer multibuf[2];
	  int num;
	  
	  num = XmbufCreateBuffers (_plotter->dpy, 
				    (Window)_plotter->drawable2, 2, 
				    MultibufferUpdateActionUndefined,
				    MultibufferUpdateHintFrequent,
				    multibuf);
	  if (num == 2)
	    /* Yow, got a pair of multibuffers.  We'll write graphics to
	       the first (`drawable3'), and interchange them on each
	       erase().  See x_erase.c. */
	    {
	      _plotter->drawable3 = multibuf[0];
	      _plotter->drawable4 = multibuf[1];	      
	      /* set double buffering type in Plotter structure */
	      _plotter->double_buffering = DBL_MBX;
	    }
	  else
	    _plotter->warning ("X server refuses to support multibuffering");
	}
    }
#endif /* HAVE_MBX_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_MULTIBUF_H */

  if (_plotter->double_buffering == DBL_NONE)
    /* user didn't request server-supported double buffering, or did but it
       couldn't be obtained */
    {
      Pixmap bg_pixmap;

      /* create background pixmap for Label widget; 2nd arg (window) is
         only used for determining the screen */
      bg_pixmap = XCreatePixmap(_plotter->dpy, 
				_plotter->drawable2,
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
	  _plotter->drawable3 = (Drawable)bg_pixmap;
	  _plotter->double_buffering = DBL_BY_HAND;
	}
      else
	{
	  _plotter->drawable1 = (Drawable)bg_pixmap;
	  _plotter->double_buffering = DBL_NONE;
	}
    }

  /* flag Plotter as open (and as having been opened at least once) */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

  /* this is initial frame of this page */
  _plotter->frame_number = 0;

  /* Create an initial drawing state with default attributes, including an
     X GC.  The drawing state won't be ready for drawing graphics, since it
     won't contain an X font or a meaningful line width.  To retrieve an X
     font and set the line width, the user will need to invoke space()
     after openpl(). */
  _plotter->savestate ();
  
  /* If there's a user-specified background color, set it in drawing state.
     This looks redundant (background color was set above), but is not. */
  bg_color_name_s = (const char *)_get_plot_param (_plotter, "BG_COLOR");
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
     buffer. */
  if (_plotter->double_buffering != DBL_NONE) 
    _plotter->erase ();

  if (_plotter->double_buffering == DBL_NONE
      || _plotter->double_buffering == DBL_BY_HAND)
    /* have a pixmap, so install it as Label widget's background pixmap */
    {
      Pixmap bg_pixmap;
      
      bg_pixmap = ((_plotter->double_buffering == DBL_BY_HAND) ? 
		   _plotter->drawable3 : _plotter->drawable1);
#ifdef USE_MOTIF
      XtSetArg (wargs[0], XmNlabelPixmap, bg_pixmap);
      XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
      XtSetValues (_plotter->canvas, wargs, (Cardinal)2);
#else
      XtSetArg (wargs[0], XtNbitmap, bg_pixmap);
      XtSetValues (_plotter->canvas, wargs, (Cardinal)1);
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
