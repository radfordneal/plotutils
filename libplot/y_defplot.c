/* This file defines the initialization for any XDrawablePlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API.

   This is identical to the initialization for an XPlotter, except that
   _y_openpl and _y_closepl replace _x_openpl and _x_closepl.  (An
   XDrawablePlotter has a Drawable passed to it as a driver parameter.) */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

const Plotter _X_drawable_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _y_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _x_endpath,  _x_erase, _x_farc, _g_farcrel, _x_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _x_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_filltype, _g_flabelwidth, _g_fline, _g_flinerel, _g_flinewidth, _x_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _x_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _y_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _x_restorestate, _x_savestate, _g_space, _g_space2, _g_textangle,
  /* internal methods that plot strings in non-Hershey fonts */
#ifdef USE_LJ_FONTS_IN_X
  _x_falabel_other, _x_falabel_other, NULL, _x_falabel_other,
  _x_flabelwidth_other, _x_flabelwidth_other, NULL, _x_flabelwidth_other,
#else
  _x_falabel_other, NULL, NULL, _x_falabel_other,
  _x_flabelwidth_other, NULL, NULL, _x_flabelwidth_other,
#endif
  /* private low-level `retrieve font' method */
  _x_retrieve_font,
  /* private low-level `sync font' method */
  NULL,
  /* private low-level `sync line attributes' method */
  _x_set_attributes,
  /* private low-level `sync color' methods */
  _x_set_pen_color,
  _x_set_fill_color,
  _x_set_bg_color,
  /* private low-level `sync position' method */
  NULL,
  /* internal error handlers */
  _g_warning,
  _g_error,
  /* basic plotter parameters */
  PL_X11_DRAWABLE,		/* stream type */
  false,			/* open? */
  false,			/* opened? */
  0,				/* number of times opened */
  (FILE *)NULL,			/* input stream [not used] */
  (FILE *)NULL,			/* output stream [not used] */
  (FILE *)NULL,			/* error stream (if any) */
  /* NUM_DEVICE_DRIVER_PARAMETERS Plotter parameters (see g_params.h) */
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  /* capabilities */
#ifdef USE_LJ_FONTS_IN_X
  1, 1, 1, 1, 1, 1, 0, 0, 0,	/* capability flags (see extern.h) */
#else
  1, 1, 1, 1, 1, 0, 0, 0, 0,	/* capability flags (see extern.h) */
#endif
  INT_MAX,			/* hard polyline length limit */
  /* output buffers */
  NULL,				/* pointer to output buffer for current page */
  NULL,				/* pointer to output buffer for first page */
  /* associated process id's */
  NULL,				/* list of pids of forked-off processes */
  0,				/* number of pids in list */
  /* drawing state(s) */
  (State *)NULL,		/* pointer to top of drawing state stack */
  &_X_default_drawstate,	/* for initialization and resetting */
  /* dimensions */
  true,				/* bitmap display device? */
  0, 569, 569, 0,		/* range of coordinates (for a bitmap device)*/
  {0.0, 0.0, 0.0, 0.0, 0.0},	/* same, for a physical device (in inches) */
  0.0,				/* units/inch for a physical device */
  true,				/* y increases downward? */
  /* elements used by more than one device */
  MAX_UNFILLED_POLYLINE_LENGTH,	/* user-settable, for unfilled polylines */
  true,				/* position is unknown? */
  {0, 0},			/* cursor position (for a bitmap device) */
  false,			/* issued warning on font substitution? */
  false,			/* issued warning on colorname substitution? */
  false,			/* issued warning on colorname substitution? */
  false,			/* issued warning on colorname substitution? */
  /* elements specific to the metafile device driver */
  false,			/* portable, not binary output format? */
  /* elements specific to the Tektronix device driver */
  D_GENERIC,			/* which sort of Tektronix? */
  MODE_ALPHA,			/* one of MODE_* */
  L_SOLID,			/* one of L_* */
  true,				/* mode is unknown? */
  true,				/* line type is unknown? */
  ANSI_SYS_GRAY30,		/* MS-DOS kermit's fg color */
  ANSI_SYS_WHITE,		/* MS-DOS kermit's bg color */
  /* elements specific to the HP-GL device driver */
  2,				/* version, 0=HP-GL, 1=HP7550A, 2=HP-GL/2 */
  0,				/* HP-GL rotation angle */
  0.0, 8128.0,			/* scaling point P1 in native HP-GL coors */
  0.0, 8128.0,			/* scaling point P2 in native HP-GL coors */
  10668.0,			/* plot length (for HP-GL/2 roll plotters) */
  1,				/* current pen (initted in h_closepl.c) */
  false,			/* bad pen? (advisory, see h_color.c) */
  false,			/* pen down rather than up? */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
  HPGL_L_SOLID,			/* line type */
  HPGL_CAP_BUTT,		/* cap style for lines */
  HPGL_JOIN_MITER,		/* join style for lines */
  HPGL_FILL_SOLID_BI,		/* fill type */
  0.0,				/* percent shading (used if FILL_SHADING) */
  2,				/* pen to be assigned a color next */
  false,			/* can construct a palette? (HP-GL/2 only) */
  true,				/* pen marks sh'd be opaque? (HP-GL/2 only) */
  PCL_ROMAN_8,			/* encoding, 14=ISO-Latin-1,.. (HP-GL/2 only)*/
  0,				/* font spacing, 0=fixed, 1=not(HP-GL/2 only)*/
  0,				/* posture, 0=upright, 1=italic(HP-GL/2 only)*/
  0,				/* weight,0=normal,3=bold, etc.(HP-GL/2 only)*/
  STICK_TYPEFACE,		/* typeface, as in g_fontdb.c (HP-GL/2 only) */
  HP_ASCII,			/* old HP character set number (lower half) */
  HP_ASCII,			/* old HP character set number (upper half) */
  0,				/* char. ht., % of p2y-p1y (HP-GL/2 only) */
  0,				/* char. width, % of p2x-p1x (HP-GL/2 only) */
  0,				/* label rise, % of p2y-p1y (HP-GL/2 only) */
  0,				/* label run, % of p2x-p1x (HP-GL/2 only) */
  0,				/* tangent of character slant (HP-GL/2 only)*/
  (unsigned char)3,		/* label terminator char (^C) */
  /* elements specific to the fig device driver */
  false,			/* whether xfig display should be in metric */
  FIG_INITIAL_DEPTH,		/* fig's current value for `depth' attribute */
  0,				/* drawing priority for last-drawn object */
  0,				/* number of colors currently defined */
  /* elements specific to the Postscript/idraw device driver */
#ifndef X_DISPLAY_MISSING
  /* elements specific to the X11 and X11 Drawable device drivers */
  (Drawable)0,			/* an X drawable (e.g. a window) */
  (Drawable)0,			/* an X drawable (e.g. a pixmap) */
  (Drawable)0,			/* graphics buffer, if double buffering */
  (Fontrecord *)NULL,		/* head of list of retrieved X fonts */
  (Colorrecord *)NULL,		/* head of list of retrieved color cells */
  (Display *)NULL,		/* display */
  (Colormap)0,			/* colormap */
  DBL_NONE,			/* double buffering type (if any) */
  0,				/* number of frame in page */
  NULL,				/* label (hint to font retrieval routine) */
  /* elements specific to the X11 device driver */
  (XtAppContext)NULL,		/* application context */
  (Widget)NULL,			/* toplevel widget */
  (Widget)NULL,			/* Label widget */
  (Drawable)0,			/* used for server-side double buffering */
  false,			/* using private colormap? */
  false,			/* window(s) disappear on Plotter deletion? */
  false,			/* issued warning on color cell exhaustion? */
#endif /* X_DISPLAY_MISSING */

  /* Long arrays are positioned at the end, and are not initialized */
  /* HP-GL driver: pen_color[] and pen_defined[] arrays */
  /* FIG: fig_usercolors[] array */
  /* PS: ps_font_used[] array */
};

/* The internal `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  Return
   value indicates whether everything proceeded smoothly. */

bool
#ifdef _HAVE_PROTOS
_X_drawable_init_plotter (Plotter *plotter)
#else
_X_drawable_init_plotter (plotter)
     Plotter *plotter;
#endif
{
  Colormap *cmap_ptr;
  Display *dpy;
  Drawable *drawable_p1, *drawable_p2;
  const char *length_s;

  /* initialize the data members specifying drawables (Pixmaps or Windows),
     and the X display with which they are associated */
  dpy = (Display *)_get_plot_param (plotter, "XDRAWABLE_DISPLAY");
  if (dpy == NULL)
    {
      plotter->warning("XDRAWABLE_DISPLAY parameter is NULL, can't initialize");
      return false;
    }
  else
    plotter->dpy = dpy;
  drawable_p1 = (Drawable *)_get_plot_param (plotter, "XDRAWABLE_DRAWABLE1");
  drawable_p2 = (Drawable *)_get_plot_param (plotter, "XDRAWABLE_DRAWABLE2");

  /* we allow either or both of the drawables to be NULL, i.e. not set */
  plotter->drawable1 = drawable_p1 ? *drawable_p1 : 0;
  plotter->drawable2 = drawable_p2 ? *drawable_p2 : 0;

  /* initialize maximum lengths for polylines */
  length_s = (const char *)_get_plot_param (plotter, "MAX_LINE_LENGTH");
  {
    int local_length;
	
    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	plotter->warning("bad MAX_LINE_LENGTH parameter, can't initialize");
	return false;
      }
    else
      plotter->max_unfilled_polyline_length = local_length;
  }

  /* find out how long polylines can get on this display */
  plotter->hard_polyline_length_limit = 
    XMaxRequestSize(plotter->dpy) / 2;

  /* allow user to specify a non-default colormap */
  cmap_ptr = (Colormap *)_get_plot_param (plotter, "XDRAWABLE_COLORMAP");
  if (cmap_ptr != NULL)
    plotter->cmap = *cmap_ptr;
  else
    {
      int screen;		/* screen number */
      Screen *screen_struct;	/* screen structure */

      /* assume colormap is default screen's colormap */
      screen = DefaultScreen (plotter->dpy);
      screen_struct = ScreenOfDisplay (plotter->dpy, screen);
      plotter->cmap = DefaultColormapOfScreen (screen_struct);
    }
  
  return true;
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  (See api.c.)  It may do such
   things as write to an output stream from internal storage, deallocate
   storage, etc.  Return value indicates whether everything went
   smoothly. */

bool
#ifdef _HAVE_PROTOS
_X_drawable_terminate_plotter (Plotter *plotter)
#else
_X_drawable_terminate_plotter (plotter)
     Plotter *plotter;
#endif
{
  return true;
}
