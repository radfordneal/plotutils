/* This file contains the openpl routine, which is a standard part of
   libplot.  It opens the graphics device.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

char *_libplot_suffix = "X";

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
  "xplot*geometry:      570x570",
#if USE_MOTIF
  "xplot*shadowThickness:	0",
  "xplot*highlightThickness:	0",
  "xplot*marginHeight:  0",
  "xplot*marginWidth:   0",
#else
  "xplot*internalHeight: 0",
  "xplot*internalWidth: 0",
#endif
  (String)NULL 
};

/* command-line mimicry; application programs may wish to change these */
char *_xplot_nargv[2] = { XPLOT_APP_NAME, NULL };
int _xplot_nargc = 1;

static String _xplot_translations =
"<BtnDown>:	quit()\n\
 <Key>Q:	quit()\n\
 <Key>q:	quit()";

/* meaningless for this device, but some apps assume these variables exist */
int _libplot_output_high_byte_first = 0;
int _libplot_output_is_ascii = 0;

int _libplot_have_font_metrics = 1;	
int _libplot_have_vector_fonts = 1;
int _libplot_have_ps_fonts = 1;
int _libplot_have_wide_lines = 1;

/* whether or not we're inside an openpl()...closepl() pair */
Boolean _grdevice_open = FALSE;

/* for storage of all X-related variables */
XData _xdata;

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static void Quit P__((Widget widget, XEvent *event, String *params, Cardinal *num_params));
#undef P__

static void			
Quit (widget, event, params, num_params) /* an action */
     Widget widget;		
     XEvent *event;
     String *params;
     Cardinal *num_params;
{
  XtDestroyWidget (_xdata.toplevel);
  XFlush (_xdata.dpy);
  exit(0);
}

/* actions; should not be modified at run time */
static XtActionsRec _xplot_actions[] = 
{
  {"quit",	Quit}
};

int
openpl ()
{
  int i, copied_nargc;
  String *copied_nargv;
  static Boolean first_time = TRUE;
  Arg wargs[10];
  /*
  XColor canvas_background;
  */
  Dimension window_height, window_width;
  unsigned long value_mask;
  XSetWindowAttributes attributes;

  if (_grdevice_open)
    {
      fprintf (stderr, "libplot: openpl() called when graphics device already open\n");
      return -1;
    }
  else
    _grdevice_open = TRUE;

  /* To permit openpl()--closepl() to be called repeatedly, we don't begin
     by calling XtAppInitialize(), since it calls XtToolkitInitialize()
     (which shouldn't be called more than once).  Instead we successively
     call XtToolkitInitialize, XtCreateApplicationContext,
     XtAppSetFallbackResources, XtOpenDisplay, and XtAppCreateShell. */

  if (first_time)
    {
      XtToolkitInitialize();
      first_time = FALSE;
    }

  _xdata.app_con = XtCreateApplicationContext();
  if (_xdata.app_con == (XtAppContext)NULL)
    {
      fprintf (stderr, "libplotX: can't create application context\n");
      _grdevice_open = FALSE;
      return -2;
    }
  XtAppSetFallbackResources (_xdata.app_con, _xplot_resources);
  
  copied_nargv = (String *)_plot_xmalloc((_xplot_nargc + 1) * sizeof(String));
  for (i = 0 ; i < _xplot_nargc ; i++) 
    copied_nargv[i] = (String)(_xplot_nargv[i]);
  copied_nargv[i] = (String)NULL;
  copied_nargc = _xplot_nargc;
  
  _xdata.dpy = XtOpenDisplay (_xdata.app_con,
			      /* display_string = NULL, so take from nargv */
			      (String)NULL, 
			      /* application name = NULL, so take from nargv */
			      (String)NULL,	
			      /* application class */
			      (String)XPLOT_APP_CLASS, 
			      /* application-specific commandline parsetable
				 (for XrmParseCommand), to set display
				 resources */
			      NULL, (Cardinal)0, 
			      &copied_nargc, copied_nargv);
  if (_xdata.dpy == (Display *)NULL)
    {
      fprintf (stderr, "libplotX: can't open X display\n");
      _grdevice_open = FALSE;
      return -2;
    }

  /* find out how long polylines can get */
  _hard_polyline_length_limit = XMaxRequestSize(_xdata.dpy) / 2;

  /* select display's default screen */
  _xdata.screen = DefaultScreen (_xdata.dpy);
  _xdata.Screen = ScreenOfDisplay (_xdata.dpy, _xdata.screen);
  _xdata.cmap = DefaultColormapOfScreen (_xdata.Screen);

  XtSetArg(wargs[0], XtNscreen, _xdata.Screen);
  XtSetArg(wargs[1], XtNargc, copied_nargc);
  XtSetArg(wargs[2], XtNargv, copied_nargv);

  /* create toplevel widget (could vary name of app instance) */
  _xdata.toplevel = XtAppCreateShell(NULL, /* name of app instance */
				     (String)XPLOT_APP_CLASS, /* app class */
				     applicationShellWidgetClass, 
				     _xdata.dpy, /* dpy to get resources from */
				     wargs, (Cardinal)3); /*arglist to get res. from*/
  free (copied_nargv);

  /* register the new action(s), and compile translation table */
  XtAppAddActions (_xdata.app_con,
		   _xplot_actions, XtNumber (_xplot_actions));

  /* create drawing canvas (a Label widget), and realize both widgets */
#if USE_MOTIF
  _xdata.canvas = XtCreateManagedWidget ((String)"", xmLabelWidgetClass,
					_xdata.toplevel, NULL, (Cardinal)0);
#else
  _xdata.canvas = XtCreateManagedWidget ((String)"", labelWidgetClass,
					_xdata.toplevel, NULL, (Cardinal)0);
#endif
  XtRealizeWidget (_xdata.toplevel);
  
  /* replace the Label widget's default translations by ours */
#if USE_MOTIF
  XtSetArg (wargs[0], XmNtranslations, 
	    XtParseTranslationTable(_xplot_translations));
#else
  XtSetArg (wargs[0], XtNtranslations, 
	    XtParseTranslationTable(_xplot_translations));
#endif
  XtSetValues (_xdata.canvas, wargs, (Cardinal)1);

  /* get the size of the Label widget's window */
  _xdata.window = XtWindow (_xdata.canvas);
#if USE_MOTIF
  XtSetArg (wargs[0], XmNwidth, &window_width);
  XtSetArg (wargs[1], XmNheight, &window_height);
#else
  XtSetArg (wargs[0], XtNwidth, &window_width);
  XtSetArg (wargs[1], XtNheight, &window_height);
#endif
  XtGetValues (_xdata.canvas, wargs, (Cardinal)2);
  _xdata.width = (unsigned int)window_width;
  _xdata.height = (unsigned int)window_height;

  /* request backing store for it */
  _xdata.backing_store = DoesBackingStore(_xdata.Screen);
  if (_xdata.backing_store)
    {
      attributes.backing_store = Always;
      value_mask = CWBackingStore;
      XChangeWindowAttributes (_xdata.dpy, _xdata.window, 
			       value_mask, &attributes);
    }

  /* create background pixmap for the Label widget */
  _xdata.pixmap = XCreatePixmap (_xdata.dpy, 
				 _xdata.window,
				 (unsigned int)window_width, 
				 (unsigned int)window_height, 
				 (unsigned int)PlanesOfScreen(_xdata.Screen));

  /* background/foreground colors for our initial drawing state */
  _default_drawstate.x_bgcolor = WhitePixelOfScreen(_xdata.Screen);
  _default_drawstate.x_fgcolor = BlackPixelOfScreen(_xdata.Screen);
  
  /* get the background color of the Label widget's window (user-settable) */
  /*
  XtSetArg (wargs[0], XtNbackground, &canvas_background);
  XtGetValues (_xdata.canvas, wargs, (Cardinal)1);
  _default_drawstate.x_bgcolor = canvas_background.pixel;
  */

  /* create an initial drawing state, with default attributes (including
     the just-computed foreground and background colors, and an X GC) */
  savestate ();
  
  /* erase pixmap by filling it with the background color, via
     XFillRectangle (need the just-created GC, and the
     _drawstate->transform struct, for this) */
  erase ();
  
  /* install pixmap in the canvas widget */
#if USE_MOTIF
  XtSetArg (wargs[0], XmNlabelPixmap, _xdata.pixmap);
  XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
  XtSetValues (_xdata.canvas, wargs, (Cardinal)2);
#else
  XtSetArg (wargs[0], XtNbitmap, _xdata.pixmap);
  XtSetValues (_xdata.canvas, wargs, (Cardinal)1);
#endif

  return 0;
}
