/* This file defines the initialization for any MetaPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

const Plotter _meta_default_plotter = 
{
  /* methods */
  _m_alabel, _m_arc, _m_arcrel, _m_bgcolor, _g_bgcolorname, _m_box, _m_boxrel, _m_capmod, _m_circle, _m_circlerel, _m_closepl, _g_color, _g_colorname, _m_cont, _m_contrel, _m_ellarc, _m_ellarcrel, _m_ellipse, _m_ellipserel, _m_endpath, _m_erase, _m_farc, _m_farcrel, _m_fbox, _m_fboxrel, _m_fcircle, _m_fcirclerel, _m_fconcat, _m_fcont, _m_fcontrel, _m_fellarc, _m_fellarcrel, _m_fellipse, _m_fellipserel, _m_ffontname, _m_ffontsize, _m_fillcolor, _g_fillcolorname, _m_filltype, _g_flabelwidth, _m_fline, _m_flinerel, _m_flinewidth, _g_flushpl, _m_fmarker, _m_fmarkerrel, _m_fmove, _m_fmoverel, _m_fontname, _m_fontsize, _m_fpoint, _m_fpointrel, _g_frotate, _g_fscale, _m_fspace, _m_fspace2, _m_ftextangle, _g_ftranslate, _g_havecap, _m_joinmod, _m_label, _g_labelwidth, _m_line, _m_linemod, _m_linerel, _m_linewidth, _m_marker, _m_markerrel, _m_move, _m_moverel, _m_openpl, _g_outfile, _m_pencolor, _g_pencolorname, _m_point, _m_pointrel, _m_restorestate, _m_savestate, _m_space, _m_space2, _m_textangle,
  /* internal methods that plot strings in non-Hershey fonts */
  NULL, NULL, NULL, NULL,
  _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, NULL,
  /* internal `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  NULL,
  /* private low-level `sync line attributes' method */
  NULL,
  /* private low-level `sync color' methods */
  NULL,
  NULL,
  NULL,
  /* private low-level `sync position' method */
  NULL,
  /* internal `error handler' methods */
  _g_warning,
  _g_error,
  /* basic plotter parameters */
  PL_META,			/* plotter type */
  false,			/* open? */
  false,			/* opened? */
  0,				/* number of times opened */
  false,			/* has space() been invoked on this page? */
  (FILE *)NULL,			/* input stream [not used] */
  (FILE *)NULL,			/* output stream (if any) */
  (FILE *)NULL,			/* error stream (if any) */
  /* NUM_DEVICE_DRIVER_PARAMETERS Plotter parameters (see g_params.h) */
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL },
  /* capabilities */
  2, 2, 2, 1, 1, 1, 1, 1,	/* capability flags (see extern.h) */
  true,				/* display device can justify text? */
  false,			/* can mix arcs and lines in stored paths? */
  AS_NONE,			/* allowed scaling for circular arcs */
  AS_NONE,			/* allowed scaling for elliptic arcs */
  INT_MAX,			/* hard polyline length limit */
  /* output buffers */
  NULL,				/* pointer to output buffer for current page */
  NULL,				/* pointer to output buffer for first page */
  /* associated process id's */
  NULL,				/* list of pids of forked-off processes */
  0,				/* number of pids in list */
  /* drawing state(s) */
  (State *)NULL,		/* pointer to top of drawing state stack */
  &_meta_default_drawstate,	/* for initialization and resetting */
  /* dimensions */
  false,			/* bitmap display device? */
  0, 0, 0, 0,			/* range of coordinates (for a bitmap device)*/
  {0.0, 0.0, 0.0, 0.0, 0.0},	/* same, for a physical device (in inches) */
  NULL,				/* page type, for a physical device */
  0.0,				/* units/inch for a physical device */
  false,			/* whether display should be in metric */
  false,			/* y increases downward? */
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
  false,			/* can construct a palette? (HP-GL/2 only) */
  true,				/* pen marks sh'd be opaque? (HP-GL/2 only) */
  1,				/* current pen (initted in h_openpl.c) */
  false,			/* bad pen? (advisory, see h_color.c) */
  false,			/* pen down rather than up? */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
  HPGL_L_SOLID,			/* line type */
  HPGL_CAP_BUTT,		/* cap style for lines */
  HPGL_JOIN_MITER,		/* join style for lines */
  HPGL_FILL_SOLID_BI,		/* fill type */
  0.0,				/* percent shading (used if FILL_SHADING) */
  2,				/* pen to be assigned a color next */
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
  /* elements specific to the fig device driver */
  FIG_INITIAL_DEPTH,		/* fig's current value for `depth' attribute */
  0,				/* number of colors currently defined */
  /* elements specific to the Postscript/idraw device driver */
  /* elements specific to the Adobe Illustrator device driver */
  AI_VERSION_5,			/* version of Illustrator file format */
  0.0, 0.0, 0.0, 1.0,		/* pen color (subtractive, in CMYK space) */
  0.0, 0.0, 0.0, 1.0,		/* fill color (subtractive, in CMYK space) */
  false, false, false, false,	/* CMYK have been used? */
  PS_CAP_BUTT,			/* PS cap style for lines */
  PS_JOIN_MITER,		/* PS join style for lines */
  L_SOLID,			/* AI's line type */
  1.0,				/* line width in printer's points */
#ifndef X_DISPLAY_MISSING
  /* elements specific to the X11 and X11 Drawable device drivers */
  (Display *)NULL,		/* display */
  (Drawable)0,			/* an X drawable (e.g. a window) */
  (Drawable)0,			/* an X drawable (e.g. a pixmap) */
  (Drawable)0,			/* graphics buffer, if double buffering */
  DBL_NONE,			/* double buffering type (if any) */
  (Fontrecord *)NULL,		/* head of list of retrieved X fonts */
  (Colorrecord *)NULL,		/* head of list of retrieved color cells */
  (Colormap)0,			/* colormap */
  0,				/* number of frame in page */
  NULL,				/* label (hint to font retrieval routine) */
  /* elements specific to the X11 device driver */
  (XtAppContext)NULL,		/* application context */
  (Widget)NULL,			/* toplevel widget */
  (Widget)NULL,			/* Label widget */
  (Drawable)0,			/* used for server-side double buffering */
  false,			/* window(s) disappear on Plotter deletion? */
  false,			/* using private colormap? */
  false,			/* issued warning on color cell exhaustion? */
#endif /* X_DISPLAY_MISSING */

  /* Long arrays are positioned at the end, and are not initialized */
  /* HP-GL driver: pen_color[] and pen_defined[] arrays */
  /* FIG: fig_usercolors[] array */
};

/* The internal `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  Return
   value indicates whether everything proceeded smoothly. */

bool
#ifdef _HAVE_PROTOS
_meta_init_plotter (Plotter *plotter)
#else
_meta_init_plotter (plotter)
     Plotter *plotter;
#endif
{
  const char *portable_s, *version_s;

  /* initialize certain data members from values of relevant class
     variables */
  portable_s = (const char *)_get_plot_param (plotter, "META_PORTABLE");
  if (strcasecmp (portable_s, "yes") == 0)
    plotter->portable_output = true;
  else
    plotter->portable_output = false;

   /* Kludge to ensure that if HPGL_VERSION="1.5", the Stick font widths
      (which differ between pre-HP-GL/2 and HP-GL/2) will be what they
      should be, when computed by the code in h_alab_pcl.c.  Yes, this is
      ugly (it shouldn't be here). */
  version_s = (const char *)_get_plot_param (plotter, "HPGL_VERSION");
  if (strcmp (version_s, "1.5") == 0) /* HP7550A */
    plotter->hpgl_version = 1;
  else if (strcmp (version_s, "1") == 0) /* generic HP-GL */
    plotter->hpgl_version = 0;

  return true;
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  (See api.c.)  It may do such
   things as write to an output stream from internal storage, deallocate
   storage, etc.  Return value indicates whether everything went
   smoothly. */

bool
#ifdef _HAVE_PROTOS
_meta_terminate_plotter (Plotter *plotter)
#else
_meta_terminate_plotter (plotter)
     Plotter *plotter;
#endif
{
  return true;
}
