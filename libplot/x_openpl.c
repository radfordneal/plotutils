/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. 

   This implementation is for XPlotters. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define XPLOT_APP_NAME "xplot"
#define XPLOT_APP_CLASS "Xplot"

/* The size of the plotting window is set here, not in extern.h.  Users can
   specify a geometry manually, in their .Xdefaults files (by specifying
   the Xplot.geometry resource, which by default is "570x570").

   The foreground color resource is ignored, since we always set the
   default pen color to black.  Users may wish to select a background color
   for the window, e.g., some sort of off-white.  That is not currently
   supported; background color is always white. */
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

/* command-line mimicry; we punch in the display string below */
const char *_xplot_nargv[4] = { XPLOT_APP_NAME, "-display", NULL, NULL };
int _xplot_nargc = 3;
#define NARGV_DISPLAY_SLOT 2

/* translate mouse clicks to `quit if closed', i.e. quit if already forked
   off, and not otherwise */
static String _xplot_translations =
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
  const char *length_s, *vanish_s;
  int i, copied_nargc;
  String *copied_nargv;
  static bool toolkit_initted = false;
  Arg wargs[10];
  Dimension window_height, window_width;
  int screen;			/* screen number */
  Screen *screen_struct;	/* screen structure */
  /*
  XColor canvas_background;
  */

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
  
  /* create new application context for this and all other XPlotters */
  _plotter->app_con = XtCreateApplicationContext();
  if (_plotter->app_con == (XtAppContext)NULL)
    {
      _plotter->error ("can't create X application context");
      return -2;
    }
  /* set misc. resources (incl. window size) to be used by our canvas
     widgets; these are specific to the application context */
  XtAppSetFallbackResources (_plotter->app_con, _xplot_resources);
  
  /* register an action table [currently containing only
     "quit_if_closed->Quit_if_closed", see above] */
  XtAppAddActions (_plotter->app_con, 
		   _xplot_actions, XtNumber (_xplot_actions));
  
  /* copy fake command-line option list */
  copied_nargv = (String *)_plot_xmalloc((_xplot_nargc + 1) * sizeof(String));
  for (i = 0 ; i < _xplot_nargc ; i++) 
    copied_nargv[i] = (String)(_xplot_nargv[i]);
  copied_nargv[i] = (String)NULL;
  copied_nargc = _xplot_nargc;
  
  /* Punch the DISPLAY parameter into the fake command-line option list.
     Incidentally DISPLAY doesn't change over the life of a Plotter. */
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
	copied_nargv[NARGV_DISPLAY_SLOT] = (String)copied_display_s;
      }
  
  /* open new connection to the X display, using fake option list */
  _plotter->dpy = 
    XtOpenDisplay (_plotter->app_con,
		   /* display_string = NULL, so take from nargv */
		   (String)NULL, 
		   /* application name = NULL, so take from nargv */
		   (String)NULL,	
		   /* application class */
		   (String)XPLOT_APP_CLASS, 
		   /* application-specific commandline
		      parsetable (for XrmParseCommand), used in
		      setting display resources */
		   NULL, (Cardinal)0, 
		   /* command line */
		   &copied_nargc, copied_nargv);
  if (_plotter->dpy == (Display *)NULL)
    {
      char *buf;
	  
      buf = (char *)_plot_xmalloc (sizeof (copied_nargv[NARGV_DISPLAY_SLOT]) + 50);
      sprintf (buf, "can't open X display \"%s\"",
	       (char *)copied_nargv[NARGV_DISPLAY_SLOT]);
      _plotter->warning (buf);
      free (buf);
      return -2;
    }
  
  /* With each connection to the display, reset list of retrieved X fonts. */
  /* Should free the previous list to avoid a memory leak; FIXME. */
  _plotter->fontlist = (Fontrecord *)NULL;

  /* find out how long polylines can get */
  _plotter->hard_polyline_length_limit = 
    XMaxRequestSize(_plotter->dpy) / 2;
  
  /* determine display's default screen, colormap */
  screen = DefaultScreen (_plotter->dpy);
  screen_struct = ScreenOfDisplay (_plotter->dpy, screen);
  _plotter->cmap = DefaultColormapOfScreen (screen_struct);
  
  /* Also for every invocation of openpl(), we create a toplevel widget,
     associated with default screen of the opened display.  (N.B. could
     vary name of app instance from invocation to invocation.) */
  XtSetArg(wargs[0], XtNscreen, screen_struct);
  XtSetArg(wargs[1], XtNargc, copied_nargc);
  XtSetArg(wargs[2], XtNargv, copied_nargv);
  _plotter->toplevel = XtAppCreateShell(NULL, /* name of app instance */
			     (String)XPLOT_APP_CLASS, /* app class */
			     applicationShellWidgetClass, 
			     _plotter->dpy, /* dpy to get resources from */
			     wargs, (Cardinal)3); /*arglist to get res. from */
  free (copied_nargv);

  /* create drawing canvas (a Label widget), and realize both widgets */
#ifdef USE_MOTIF
  _plotter->canvas = XtCreateManagedWidget ((String)"", xmLabelWidgetClass,
					_plotter->toplevel, NULL, (Cardinal)0);
#else
  _plotter->canvas = XtCreateManagedWidget ((String)"", labelWidgetClass,
					_plotter->toplevel, NULL, (Cardinal)0);
#endif
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

  /* get Label widget's window and window size.  Store it in Plotter struct
     as `drawable #2'. */
  _plotter->drawable2 = (Drawable)XtWindow(_plotter->canvas);

#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNwidth, &window_width);
  XtSetArg (wargs[1], XmNheight, &window_height);
#else
  XtSetArg (wargs[0], XtNwidth, &window_width);
  XtSetArg (wargs[1], XtNheight, &window_height);
#endif
  XtGetValues (_plotter->canvas, wargs, (Cardinal)2);
  _plotter->imin = 0;
  _plotter->imax = window_width - 1;
  /* note flipped-y convention for this device: min > max */
  _plotter->jmin = window_height - 1;
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

  /* Create background pixmap for Label widget; 2nd arg specifies screen.
     Store it in Plotter struct as `drawable #1'. */
  _plotter->drawable1 
    = (Drawable)XCreatePixmap(_plotter->dpy, 
			      _plotter->drawable2,
			      (unsigned int)window_width, 
			      (unsigned int)window_height, 
			      (unsigned int)PlanesOfScreen(screen_struct));

  /* save background/foreground colors for initial drawing state; store in
     default drawing state because there's no real drawing state in the
     XPlotter yet */
  _plotter->default_drawstate->x_bgcolor = WhitePixelOfScreen(screen_struct);
  _plotter->default_drawstate->x_fgcolor = BlackPixelOfScreen(screen_struct);
  
  /* get background color of the Label widget's window (user-settable) */
  /*
  XtSetArg (wargs[0], XtNbackground, &canvas_background);
  XtGetValues (_plotter->canvas, wargs, (Cardinal)1);
  _plotter->default_drawstate->x_bgcolor = canvas_background.pixel;
  */

  /* flag Plotter as open (and as having been opened at least once) */
  _plotter->open = true;
  _plotter->opened = true;

  /* Create an initial drawing state with default attributes, including the
     just-computed foreground and background colors, and an X GC.  The
     drawing state won't be fully ready for drawing graphics, since it
     won't contain an X font.  To retrieve the initial X font, the user
     will need to invoke space() after openpl(). */
  _plotter->savestate ();
  
  /* erase pixmap and window by filling them with the background color, via
     XFillRectangle (the just-created drawing state, with the mentioned
     attributes, is used for this) */
  _plotter->erase ();
  
  /* install pixmap as the Label widget's background pixmap */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNlabelPixmap, (Pixmap)_plotter->drawable1);
  XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
  XtSetValues (_plotter->canvas, wargs, (Cardinal)2);
#else
  XtSetArg (wargs[0], XtNbitmap, (Pixmap)_plotter->drawable1);
  XtSetValues (_plotter->canvas, wargs, (Cardinal)1);
#endif

  /* do an XSync on the display (this will cause the background color of
   the pixmap to show up) */
  _plotter->flushpl ();

  return 0;
}
