/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. 

   This implementation is for XPlotters. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define XPLOT_APP_NAME "xplot"
#define XPLOT_APP_CLASS "Xplot"

/* Fallback resources.  The default size of the plotting window is set
   here, not in extern.h.  Users may override the default by specifying a
   geometry manually, in their .Xdefaults files (by specifying the
   xplot.geometry resource), and of course by specifying the device driver
   parameter BITMAPSIZE.

   Note that the foreground and background color resources are ignored,
   since we always initialize them to black and white respectively. */
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

/* command-line mimicry; we punch in the display and geometry */
static char *_fake_argv[] = { XPLOT_APP_NAME, "-display", NULL, "-geometry", NULL, NULL };
#define FAKE_ARGV_DISPLAY_SLOT 2
#define FAKE_ARGV_GEOMETRY_SLOT 4
#define DEFAULT_FAKE_ARGC 3	/* assuming -geometry option not included */

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
      exit (0);
    }
}

/* application context-specific action table */
static XtActionsRec _xplot_actions[] = 
{
  {"quit_if_closed",	Quit_if_closed}
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
  Pixmap bg_pixmap;
  Screen *screen_struct;	/* screen structure */
  String *copied_fake_argv;
  const char *length_s, *vanish_s, *bg_color_name_s, *double_buffer_s;
  int i, fake_argc, copied_fake_argc;
  int screen;			/* screen number */
  static bool toolkit_initted = false;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* If Plotter hasn't been opened before, initialize certain data members
     from parameters; these don't change over the life of a Plotter. */
  if (!_plotter->opened)
    {
      vanish_s = (const char *)_get_plot_param ("VANISH_ON_DELETE");
      if (strcasecmp (vanish_s, "yes") == 0)
	_plotter->vanish_on_delete = true;
      else
	_plotter->vanish_on_delete = false;
      
      length_s = (const char *)_get_plot_param ("MAX_LINE_LENGTH");
      {
	int local_length;
	
	if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
	  {
	    _plotter->error("bad MAX_LINE_LENGTH parameter, can't initialize");
	    return -1;
	  }
	else
	  _plotter->max_unfilled_polyline_length = local_length;
      }
    }

  /* To permit openpl..closepl to be invoked repeatedly, we don't begin by
     calling XtAppInitialize(), since it calls XtToolkitInitialize() (which
     shouldn't be called more than once).  Instead we successively call
     XtToolkitInitialize, XtCreateApplicationContext,
     XtAppSetFallbackResources, XtOpenDisplay, and XtAppCreateShell.  All
     but the first are called once per invocation of openpl().  That looks
     wasteful (we set up a new application context each time.  But since a
     forked-off process will manage the window drawn by the
     openpl..closepl, it's appropriate. */

  if (!toolkit_initted)
    {
      XtToolkitInitialize();
      toolkit_initted = true;
    }
  
  /* create new application context for this XPlotter page */
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
  
  /* Punch the DISPLAY parameter into the fake command-line option list.
     Incidentally, DISPLAY doesn't change over the life of a Plotter,
     i.e. between invocations of openpl(). */
      {
	const char *display_s;
	char *copied_display_s;
	
	display_s = (const char *)_get_plot_param ("DISPLAY");
	if (display_s == NULL)	/* shouldn't happen */
	  {
	    _plotter->error ("can't open null X Window System display");
	    return -1;
	  }
	copied_display_s = (char *)_plot_xmalloc(strlen (display_s) + 1);
	strcpy (copied_display_s, display_s);
	_fake_argv[FAKE_ARGV_DISPLAY_SLOT] = (String)copied_display_s;
	fake_argc = DEFAULT_FAKE_ARGC;
      }
  
  /* check whether user specified BITMAPSIZE, if so use it as -geometry arg */
      {
	const char *bitmap_size_s;
	char *copied_bitmap_size_s;
	
	bitmap_size_s = (const char *)_get_plot_param ("BITMAPSIZE");
	if (bitmap_size_s && _bitmap_size_ok (bitmap_size_s))
	  {
	    copied_bitmap_size_s = (char *)_plot_xmalloc(strlen (bitmap_size_s) + 1);
	    strcpy (copied_bitmap_size_s, bitmap_size_s);
	    _fake_argv[FAKE_ARGV_GEOMETRY_SLOT] = (String)copied_bitmap_size_s;
	    fake_argc += 2;	/* -geometry and its arg */
	  }
      }

  /* copy fake command-line option list */
  copied_fake_argv = (String *)_plot_xmalloc((fake_argc + 1) * sizeof(String));
  for (i = 0 ; i < fake_argc ; i++) 
    copied_fake_argv[i] = (String)(_fake_argv[i]);
  copied_fake_argv[i] = (String)NULL;
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
      char *buf;
	  
      buf = (char *)_plot_xmalloc (sizeof (copied_fake_argv[FAKE_ARGV_DISPLAY_SLOT]) + 50);
      sprintf (buf, "can't open X display \"%s\"",
	       (char *)copied_fake_argv[FAKE_ARGV_DISPLAY_SLOT]);
      _plotter->warning (buf);
      free (buf);
      return -2;
    }
  
  /* find out how long polylines can get */
  _plotter->hard_polyline_length_limit = 
    XMaxRequestSize(_plotter->dpy) / 2;
  
  /* determine display's default screen, colormap */
  screen = DefaultScreen (_plotter->dpy);
  screen_struct = ScreenOfDisplay (_plotter->dpy, screen);
  _plotter->cmap = DefaultColormapOfScreen (screen_struct);
  
  /* Also for every invocation of openpl(), we create a toplevel widget,
     associated with default screen of the opened display.  (N.B. could
     vary name of app instance from page to page.) */
  XtSetArg(wargs[0], XtNscreen, screen_struct);
  XtSetArg(wargs[1], XtNargc, copied_fake_argc);
  XtSetArg(wargs[2], XtNargv, copied_fake_argv);
  _plotter->toplevel = XtAppCreateShell(NULL, /* name of app instance */
			     (String)XPLOT_APP_CLASS, /* app class */
			     applicationShellWidgetClass, 
			     _plotter->dpy, /* dpy to get resources from */
			     wargs, (Cardinal)3); /*arglist to get res. from */
  free (copied_fake_argv);

  /* Create drawing canvas (a Label widget) as child of toplevel widget.
     If user specified BITMAPSIZE parameter we use it; otherwise size taken
     from xplot.geometry.  Fallback size is specified at head of file. */
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

  /* create background pixmap for Label widget; 2nd arg determines screen */
  bg_pixmap = XCreatePixmap(_plotter->dpy, 
			     _plotter->drawable2,
			     (unsigned int)window_width, 
			     (unsigned int)window_height, 
			     (unsigned int)PlanesOfScreen(screen_struct));
  
  /* if user requested double buffering, use this pixmap as the only
     drawable into which we'll draw; otherwise use it as the 2nd of two
     drawables into which we'll draw, the other being the window */
  double_buffer_s = (const char *)_get_plot_param ("USE_DOUBLE_BUFFERING");
  if (strcmp (double_buffer_s, "yes") == 0)
    {
      _plotter->drawable3 = (Drawable)bg_pixmap;
      _plotter->double_buffering = true;
    }
  else
    _plotter->drawable1 = (Drawable)bg_pixmap;

  /* flag Plotter as open (and as having been opened at least once) */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* this is initial frame of this page */
  _plotter->frame_number = 0;

  /* Create an initial drawing state with default attributes, including an
     X GC.  The drawing state won't be ready for drawing graphics, since it
     won't contain an X font or a meaningful line width.  To retrieve an X
     font and set the line width, the user will need to invoke space()
     after openpl(). */
  _plotter->savestate ();
  
  /* if there's a user-specified background color, set it in drawing state */
  bg_color_name_s = (const char *)_get_plot_param ("BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (bg_color_name_s);
  
  /* clear both pixmap and window by filling them with the background
     color, via XFillRectangle (the just-created drawing state, with the
     mentioned attributes, is used for this) */
  _plotter->erase ();
  
  /* If double buffering, must invoke `erase' one more time to clear both
     pixmap and window, since what `erase' does in that case is (1) copy
     pixmap to window, and (2) clear pixmap. */
  if (_plotter->double_buffering)
    _plotter->erase ();

  /* install pixmap as the Label widget's background pixmap */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNlabelPixmap, (Pixmap)bg_pixmap);
  XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
  XtSetValues (_plotter->canvas, wargs, (Cardinal)2);
#else
  XtSetArg (wargs[0], XtNbitmap, (Pixmap)bg_pixmap);
  XtSetValues (_plotter->canvas, wargs, (Cardinal)1);
#endif

  /* do an XSync on the display (this will cause the background color of
   the pixmap to show up) */
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
      && (sscanf (bitmap_size_s, "%dx%d", &width, &height) == 2)
      && (width > 0) && (height > 0))
    return true;
  else
    return false;
}
