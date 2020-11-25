/* This file defines the initialization for any PSPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* Note that we define the graphics display to be a square, centered on the
   printed page and occupying the full width of the page. */

/* The size of the graphics display is determined by the PAGESIZE
   environment variable ("usletter", "a4", etc.)  The table of known
   pagetypes is in pagetype.h.  The default is "usletter", for which the
   graphics display is an 8.5" by 8.5" square, centered on an 8.5" by 11"
   page. */

/* initialization for _plotter */
Plotter _ps_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _p_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _p_endpath, _p_erase, _g_farc, _g_farcrel, _g_fbox, _g_fboxrel, _p_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _p_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_filltype, _g_flabelwidth, _g_fline, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _p_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _p_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* internal methods that plot strings of various supported types */
  _p_falabel_ps, NULL, NULL,
  _g_flabelwidth_ps, NULL, NULL,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  NULL,
  /* private low-level `sync line attributes' method */
  NULL,
  /* private low-level `sync color' methods */
  _p_set_pen_color,
  _p_set_fill_color,
  /* private low-level `sync position' method */
  NULL,
  /* error handlers */
  _g_warning,
  _g_error,
  /* basic plotter parameters */
  PL_PS,			/* plotter type */
  false,			/* open? */
  false,			/* opened? */
  0,				/* number of times opened */
  (FILE *)NULL,			/* input stream [not used] */
  (FILE *)NULL,			/* output stream (if any) */
  (FILE *)NULL,			/* error stream (if any) */
  /* device driver parameters (see g_params.h) */
  { NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL },
  /* capabilities */
  1, 1, 0, 1, 1, 0, 0,		/* capability flags (see extern.h) */
  MAXINT,			/* hard polyline length limit */
  /* output buffer */
  {NULL, 0, NULL, 0},		/* output buffer (for non-realtime devices) */
  /* associated process id's */
  NULL,				/* list of pids of forked-off processes */
  0,				/* number of pids in list */
  /* drawing state(s) */
  (State *)NULL,		/* pointer to top of drawing state stack */
  &_ps_default_drawstate,	/* for initialization and resetting */
  /* dimensions */  
  false,			/* bitmap display device? */
  0, 0, 0, 0,			/* range of coordinates (for a bitmap device)*/
  {0.25, 8.25, 1.5, 9.5, 0.0},	/* same, for a physical device (in inches) */
  72.0,				/* units/inch for a physical device */
  false,			/* y increases downward? */
  /* elements used by more than one device */
  MAX_UNFILLED_POLYLINE_LENGTH,	/* user-settable, for unfilled polylines */
  true,				/* position is unknown? */
  {0, 0},			/* cursor position (for a bitmap device) */
  false,			/* issued warning on lack of sheared fonts */
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
  1,				/* current pen (initted in closepl.c) */
  false,			/* pen down rather than up? */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
  HPGL_L_SOLID,			/* line type */
  HPGL_CAP_BUTT,		/* cap style for lines */
  HPGL_JOIN_MITER,		/* join style for lines */
  HPGL_FILL_SOLID_BI,		/* fill type */
  0.0,				/* percent shading (used if FILL_SHADING) */
  false,			/* monochrome, i.e. have pen #1 only? */
  2,				/* pen to be assigned a color next */
  false,			/* can construct a palette? (HP-GL/2 only) */
  true,				/* white pen sh'd be opaque? (HP-GL/2 only) */
  PCL_ROMAN_8,			/* encoding, 14=ISO-Latin-1,.. (HP-GL/2 only)*/
  0,				/* font spacing, 0=fixed, 1=not(HP-GL/2 only)*/
  0,				/* posture, 0=upright, 1=italic(HP-GL/2 only)*/
  0,				/* weight,0=normal,3=bold, etc.(HP-GL/2 only)*/
  STICK_TYPEFACE,		/* typeface, as in g_fontdb.c (HP-GL/2 only) */
  0,				/* char. ht., % of p2y-p1y (HP-GL/2 only) */
  0,				/* char. width, % of p2x-p1x (HP-GL/2 only) */
  0,				/* label rise, % of p2y-p1y (HP-GL/2 only) */
  0,				/* label run, % of p2x-p1x (HP-GL/2 only) */
  0,				/* tangent of character slant (HP-GL/2 only)*/
  (unsigned char)3,		/* character used as label terminator (same) */
  /* elements specific to the fig device driver */
  false,			/* whether xfig display should be in metric */
  FIG_INITIAL_DEPTH,		/* fig's current value for `depth' attribute */
  0,				/* drawing priority for last-drawn object */
  0,				/* number of colors currently defined */
  /* elements specific to the Postscript/idraw device driver */
  0.0, 0.0, 0.0, 0.0,		/* bounding box elements (dummies) */
#ifndef X_DISPLAY_MISSING
  /* elements specific to the X11 and X11 Drawable device drivers */
  (Drawable)NULL,		/* an X drawable (e.g. a window) */
  (Drawable)NULL,		/* an X drawable (e.g. a pixmap) */
  (Fontrecord *)NULL,		/* head of list of retrieved X fonts */
  (Display *)NULL,		/* display */
  (Colormap)NULL,		/* colormap */
  /* elements specific to the X11 device driver */
  (XtAppContext)NULL,		/* application context */
  (Widget)NULL,			/* toplevel widget */
  (Widget)NULL,			/* Label widget */
  false,			/* window(s) disappear on Plotter deletion? */
#endif /* X_DISPLAY_MISSING */

  /* Long arrays are positioned at the end, and are not initialized */
  /* HP-GL driver: pen_color[] and pen_defined[] arrays */
  /* FIG: fig_usercolors[] array */
  /* PS: ps_font_used[] array */
};
