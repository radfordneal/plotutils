/* This file defines the initializations for HPGLPlotter and PCLPlotter
   objects including both private data and public methods.  There is a
   one-to-one correspondence between public methods and user-callable
   functions in the C API. */

/* Currently, only differences between the two types of Plotter are (1) the
   different `type' field in corresponding _*_default_plotter structures,
   and (2) some tests for same, in h_openpl.c (in the openpl and closepl
   methods).  The differences are due to the PCL 5 control codes that must
   be emitted to switch the printer into HP-GL/2 mode, etc; also to the
   fact that HPGLPlotters emit graphics after each page, but PCLPlotters
   wait until the end of a job, when the Plotter is deleted. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* forward references */
static bool _parse_pen_string __P ((const char *pen_s, Plotter *state_ptr));
static bool _string_to_inches __P ((const char *offset_s, double *offset));

#define MAX_COLOR_NAME_LEN 32	/* long enough for all known colors */

/* The size of the graphics display is determined by the PAGESIZE
   environment variable ("usletter", "a4", etc.)  The table of known
   pagetypes is in g_pagetype.h.  The default is "usletter", for which the
   graphics display is a square with side length equal to 80% of 8.5", so
   it should fit on an 8.5" by 11" page.

   The origin of the HP-GL[/2] coordinate system is the lower left corner
   of the `hard-clip region', a rectangle that is not the same as the page.
   For this reason we allow the user to shift the position of the graphics
   display via the environment variables HPGL_XOFFSET and HPGL_YOFFSET (see
   below). */

/* This version is for HPGLPlotters; the PCLPlotter counterpart immediately
   follows. */

const Plotter _hpgl_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _h_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _h_endpath, _g_erase, _g_farc, _g_farcrel, _h_fbox, _g_fboxrel, _h_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_filltype, _g_flabelwidth, _g_fline, _g_flinerel, _h_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _h_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _h_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* internal methods that plot strings in non-Hershey fonts */
#ifdef USE_PS_FONTS_IN_PCL
  _h_falabel_pcl, _h_falabel_pcl, _h_falabel_pcl, NULL,
  _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, NULL,
#else
  NULL, _h_falabel_pcl, _h_falabel_pcl, NULL,
  NULL, _g_flabelwidth_pcl, _g_flabelwidth_stick, NULL,
#endif
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _h_set_font,
  /* private low-level `sync line attributes' method */
  _h_set_attributes,
  /* private low-level `sync color' methods */
  _h_set_pen_color,
  _h_set_fill_color,
  NULL,
  /* private low-level `sync position' method */
  _h_set_position,
  /* error handlers */
  _g_warning,
  _g_error,
  /* basic plotter parameters */
  PL_HPGL,			/* plotter type */
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
  /* capabilities (these are appropriate for HP-GL/2 [the default])*/
#ifdef USE_PS_FONTS_IN_PCL
  1, 1, 0, 1, 1, 1, 1, 0,	/* capability flags (see extern.h) */
#else
  1, 1, 0, 1, 0, 1, 1, 0,	/* capability flags (see extern.h) */
#endif
  false,			/* display device can justify text? */
  true,				/* can mix arcs and lines in stored paths? */
  AS_UNIFORM,			/* allowed scaling for circular arcs */
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
  &_hpgl_default_drawstate,	/* for initialization and resetting */
  /* dimensions */
  false,			/* bitmap display device? */
  0, 0, 0, 0,			/* range of coordinates (for a bitmap device)*/
  {0.0, 8.0, 0.0, 8.0, 10.5},	/* same, for a physical device (in inches) */
  NULL,				/* page type, for a physical device */
  (double)HPGL_UNITS_PER_INCH,	/* units/inch for a physical device */
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

/* This version is for PCL Plotters rather than HPGL Plotters.  It is
   exactly the same as the above, except that the `type' member is set to
   PL_PCL rather than PL_HPGL. */

const Plotter _pcl_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _h_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _h_endpath, _g_erase, _g_farc, _g_farcrel, _h_fbox, _g_fboxrel, _h_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_filltype, _g_flabelwidth, _g_fline, _g_flinerel, _h_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _h_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _h_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* internal methods that plot strings in non-Hershey fonts */
#ifdef USE_PS_FONTS_IN_PCL
  _h_falabel_pcl, _h_falabel_pcl, _h_falabel_pcl, NULL,
  _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, NULL,
#else
  NULL, _h_falabel_pcl, _h_falabel_pcl, NULL,
  NULL, _g_flabelwidth_pcl, _g_flabelwidth_stick, NULL,
#endif
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _h_set_font,
  /* private low-level `sync line attributes' method */
  _h_set_attributes,
  /* private low-level `sync color' methods */
  _h_set_pen_color,
  _h_set_fill_color,
  NULL,
  /* private low-level `sync position' method */
  _h_set_position,
  /* error handlers */
  _g_warning,
  _g_error,
  /* basic plotter parameters */
  PL_PCL,			/* plotter type */
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
  /* capabilities (these are appropriate for HP-GL/2 [the default])*/
#ifdef USE_PS_FONTS_IN_PCL
  1, 1, 0, 1, 1, 1, 1, 0,	/* capability flags (see extern.h) */
#else
  1, 1, 0, 1, 0, 1, 1, 0,	/* capability flags (see extern.h) */
#endif
  false,			/* display device can justify text? */
  true,				/* can mix arcs and lines in stored paths? */
  AS_UNIFORM,			/* allowed scaling for circular arcs */
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
  &_hpgl_default_drawstate,	/* for initialization and resetting */
  /* dimensions */
  false,			/* bitmap display device? */
  0, 0, 0, 0,			/* range of coordinates (for a bitmap device)*/
  {0.0, 8.0, 0.0, 8.0, 10.5},	/* same, for a physical device (in inches) */
  NULL,				/* page type, for a physical device */
  (double)HPGL_UNITS_PER_INCH,	/* units/inch for a physical device */
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

/* This initialization serves for both HPGL and PCL Plotters, which are
   almost identical.

   For HPGL Plotters, we determine the HP-GL version from the environment
   variable "HPGL_VERSION" ("1", "1.5", or "2", meaning generic HP-GL,
   HP7550A, and modern HP-GL/2 respectively), and determine the page size
   and the location on the page of the graphics display, so that we'll be
   able to work out the map from user coordinates to device coordinates in
   space.c.

   We allow the user to shift the location of the graphics display by
   specifying an offset vector, since the origin of the HP-GL coordinate
   system and the size of the `hard-clip region' within which graphics can
   be drawn are not known.  (There are so many HP-GL and HP-GL/2
   devices.)

   We also work out which pens are available, and whether the device, if an
   HP-GL/2 device, supports the Palette Extension so that new logical pens
   can be defined as RGB triples.  The HPGL_PENS and HPGL_ASSIGN_COLORS
   environment variables are used for this.  (The default is for a generic
   HP-GL device to have exactly 1 pen, #1, and for an HP7550A or HP-GL/2
   device to have 7 pens, #1 through #7, with colors equal to the seven
   non-white vertices of the RGB color cube.  We allow the user to specify
   up to 31 pens, #1 through #31, via HPGL_PENS. */

bool
#ifdef _HAVE_PROTOS
_hpgl_init_plotter (Plotter *plotter)
#else
_hpgl_init_plotter (plotter)
     Plotter *plotter;
#endif
{
  const char *version_s, *rotate_s, *xoffset_s, *yoffset_s, *pen_s, *transparent_s, *length_s, *pagesize;
  const Pagedata *pagedata;
  bool retval = true;
  int i;

  /* initialize certain data members from values of relevant class
     variables */
      
  length_s = (const char *)_get_plot_param (plotter, "MAX_LINE_LENGTH");
  {
    int local_length;
	
    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	plotter->warning ("bad MAX_LINE_LENGTH parameter, can't initialize");
	retval = false;
      }
    else
      plotter->max_unfilled_polyline_length = local_length;
  }
      
  /* if an HPGLPlotter rather than a PCLPlotter, check for HP/GL version;
     if a PCLPlotter, use default version (i.e. 2) */

  version_s = (const char *)_get_plot_param (plotter, "HPGL_VERSION");
  if (plotter->type == PL_HPGL)
    {
      if (strcmp (version_s, "1") == 0) /* generic HP-GL, HP7220 or HP7475A */
	{
	  plotter->hpgl_version = 0;
	  plotter->have_wide_lines = 0;
	  plotter->have_solid_fill = 0;
	  plotter->have_ps_fonts = 0;
	  plotter->have_pcl_fonts = 0;
	  plotter->have_stick_fonts = 0;
	  plotter->have_extra_stick_fonts = 0;
	}
      else if (strcmp (version_s, "1.5") == 0) /* HP7550A */
	{
	  plotter->hpgl_version = 1;
	  plotter->have_wide_lines = 0;
	  plotter->have_solid_fill = 1;
	  plotter->have_ps_fonts = 0;
	  plotter->have_pcl_fonts = 0;
	  plotter->have_stick_fonts = 1;
	  plotter->have_extra_stick_fonts = 1;
	}
      else if (strcmp (version_s, "2") == 0) /* HP-GL/2 */
	{
	  plotter->hpgl_version = 2;
	  plotter->have_wide_lines = 1;
	  plotter->have_solid_fill = 1;
	  /* we take `have_ps_fonts' from the default value; see capability
	     flag above, which depends on USE_PS_FONTS_IN_PCL (a config-time
	     option) */
	  plotter->have_pcl_fonts = 1;
	  plotter->have_stick_fonts = 1;
	  plotter->have_extra_stick_fonts = 0;
	}
      else 
	{
	  plotter->warning ("bad HPGL_VERSION variable, can't initialize");
	  retval = false;
	}
    }
  
  /* determine page type i.e. determine the range of device coordinates
     over which the graphics display will extend (and hence the
     transformation from user to device coordinates). */
  pagesize = (const char *)_get_plot_param (plotter, "PAGESIZE");
  pagedata = _pagetype(pagesize);
  if (pagedata == NULL)
    {
      plotter->warning ("bad PAGESIZE variable, can't initialize");
      retval = false;
    }
  plotter->display_coors = pagedata->hpgl;
      
  /* Origin of HP-GL[/2] coordinates is the lower left corner of the
     hard-clip region, which is not the same as the lower left corner
     of the page.  So we allow the user to specify an offset for the
     location of the graphics display. */
  if (plotter->type == PL_HPGL)
    {
      xoffset_s = (const char *)_get_plot_param (plotter, "HPGL_XOFFSET");
      yoffset_s = (const char *)_get_plot_param (plotter, "HPGL_YOFFSET");  
    }
  else
    /* PCL Plotter */
    {
      xoffset_s = (const char *)_get_plot_param (plotter, "PCL_XOFFSET");
      yoffset_s = (const char *)_get_plot_param (plotter, "PCL_YOFFSET");  
    }
  {
    double xoffset;
	
    if (_string_to_inches (xoffset_s, &xoffset))
      {
	plotter->display_coors.left += xoffset;
	plotter->display_coors.right += xoffset;
      }
    else
      {
	if (plotter->type != PL_HPGL)
	  /* PCL Plotter */
	  plotter->warning ("bad PCL_XOFFSET variable, can't initialize");
	else
	  plotter->warning ("bad HPGL_XOFFSET variable, can't initialize");
	retval = false;
      }
  }
  {
    double yoffset;
	
    if (_string_to_inches (yoffset_s, &yoffset))
      {
	plotter->display_coors.bottom += yoffset;
	plotter->display_coors.top += yoffset;
      }
    else
      {
	if (plotter->type != PL_HPGL)
	  /* PCL Plotter */
	  plotter->warning ("bad PCL_YOFFSET variable, can't initialize");
	else
	  plotter->warning ("bad HPGL_YOFFSET variable, can't initialize");
	retval = false;
      }
  }
      
  /* At this point we stash the coordinates for later use.  They'll become
     the coordinates of our `scaling points' P1 and P2 (see h_openpl.c).
     The numbers in our output file will be normalized device coordinates,
     not physical device coordinates (for the transformation between them,
     which is accomplished by the HP-GL `SC' instruction, see h_openpl.c) */
  plotter->p1x = plotter->display_coors.left * HPGL_UNITS_PER_INCH;
  plotter->p2x = plotter->display_coors.right * HPGL_UNITS_PER_INCH;
  plotter->p1y = plotter->display_coors.bottom * HPGL_UNITS_PER_INCH;
  plotter->p2y = plotter->display_coors.top * HPGL_UNITS_PER_INCH;
  plotter->display_coors.left = HPGL_SCALED_DEVICE_LEFT; 
  plotter->display_coors.right = HPGL_SCALED_DEVICE_RIGHT;
  plotter->display_coors.bottom = HPGL_SCALED_DEVICE_BOTTOM;
  plotter->display_coors.top = HPGL_SCALED_DEVICE_TOP;
  plotter->device_units_per_inch = 1.0;
      
  /* plot length (to be emitted in an HP-GL/2 `PS' instruction,
     important mostly for roll plotters) */
  plotter->plot_length = 
    plotter->display_coors.extra * HPGL_UNITS_PER_INCH;

  /* determine whether to rotate the figure (e.g. horizontal instead of
     vertical, see h_openpl.c) */
  if (plotter->type != PL_HPGL)
    /* PCL Plotter */
    rotate_s = (const char *)_get_plot_param (plotter, "PCL_ROTATE");
  else
    rotate_s = (const char *)_get_plot_param (plotter, "HPGL_ROTATE");
  if (strcasecmp (rotate_s, "yes") == 0
      || strcmp (rotate_s, "90") == 0)
    plotter->rotation = 90;
  else if (strcmp (rotate_s, "180") == 0 && plotter->hpgl_version == 2)
    plotter->rotation = 180;
  else if (strcmp (rotate_s, "270") == 0 && plotter->hpgl_version == 2)
    plotter->rotation = 270;
  else
    plotter->rotation = 0;
      

  if (plotter->type == PL_HPGL)
  /* HPGL Plotter, not PCL Plotter; should we avoid emitting the `white is
     opaque' HP-GL/2 instruction?  (HP-GL/2 pen plotters may not like it) */
    {
      transparent_s = (const char *)_get_plot_param (plotter, "HPGL_OPAQUE_MODE");
      if (strcasecmp (transparent_s, "no") == 0)
	plotter->opaque_mode = false;
      else
	plotter->opaque_mode = true;
    }
      
  /* do we support the HP-GL/2 palette extension, i.e. can we define
     new logical pens as RGB triples? (user must request this) */
  plotter->palette = false;
  if (plotter->hpgl_version == 2)
    {
      const char *palette_s;
	  
      if (plotter->type == PL_HPGL)
	palette_s = (const char *)_get_plot_param (plotter, "HPGL_ASSIGN_COLORS");
      else
	/* PCL Plotter */
	palette_s = (const char *)_get_plot_param (plotter, "PCL_ASSIGN_COLORS");
      if (strcasecmp (palette_s, "yes") == 0)
	plotter->palette = true;
    }
      
  /* initialize pen color array, typically 0..31 */
  for (i = 0; i < MAX_NUM_PENS; i++)
    plotter->pen_defined[i] = 0; /* pen absent */
      
  /* pen #0 (white pen, RGB=255,255,255) is always defined */
  plotter->pen_color[0].red = 255;
  plotter->pen_color[0].green = 255;
  plotter->pen_color[0].blue = 255;
  plotter->pen_defined[0] = 2; /* i.e. hard-defined */
      
  /* determine initial palette, i.e. available pens in 1..31 range */
  if (plotter->type != PL_HPGL)
    /* PCL Plotter */
    pen_s = DEFAULT_HPGL2_PEN_STRING;
  else
    {
      if ((pen_s = (const char *)_get_plot_param (plotter, "HPGL_PENS")) == NULL)
	/* since no value is assigned to HPGL_PENS by default, user must
	   not have assigned a value to it; we'll choose a value based on
	   version */
	{
	  if (plotter->hpgl_version == 0) /* i.e. generic HP-GL */
	    pen_s = DEFAULT_HPGL_PEN_STRING;
	  else
	    pen_s = DEFAULT_HPGL2_PEN_STRING;
	}
    }
  
  if (_parse_pen_string (pen_s, plotter) == false)
    {
      plotter->warning ("bad HPGL_PENS variable, can't initialize");
      retval = false;
    }
  /* if no logical pens, insist on pen #1 being present (backward
     compatibility?) */
  if (plotter->palette == false 
      && plotter->pen_defined[1] == 0)
    {
      plotter->warning ("pen #1 not defined in HPGL_PENS variable, can't initialize");
      retval = false;
    }
  /* examine presence or absence of hard-defined pens in 2..31 range */
  {
    bool have_more_than_one_pen = false;
    bool have_free_pens = false;
	
    for (i = 2; i < MAX_NUM_PENS; i++)
      {
	if (plotter->pen_defined[i] == 2)
	  /* at least one pen with number > 1 is hard-defined */
	  have_more_than_one_pen = true;
	else
	  /* at least one pen with number > 1 is not hard-defined */
	  {
	    /* record which such was encountered first */
	    if (have_free_pens == false)
	      plotter->free_pen = i;
	    have_free_pens = true;
	  }
      }
    if (!have_free_pens)	
      /* luser specified too many pens, can't soft-define colors */
      plotter->palette = false;
  }

  return retval;
}

static bool 
#ifdef _HAVE_PROTOS
_string_to_inches(const char *string, double *inches)
#else
_string_to_inches(string, inches)
     const char *string; 
     double *inches;
#endif
{
  double val;
  char s[4];
  
  if (sscanf (string, "%lf %3s", &val, s) == 2)
    {
      if (strlen (s) > 2)
	return false;
      if (strcmp (s, "in") == 0)
	{
	  *inches = val;
	  return true;
	}
      else if (strcmp (s, "cm") == 0)
	{
	  *inches = val / 2.54;
	  return true;
	}
      else if (strcmp (s, "mm") == 0)      
	{
	  *inches = val / 25.4;
	  return true;
	}
    }
    return false;
}

/* Parse a pen string, e.g. a user-specified HPGL_PENS environment
   variable, specifying which pens are available.  Result is stored in a
   specified device structure.  More pens (logical pens) may be added later
   to the array of available pens, if the plotter is an HP-GL/2 device and
   supports the palette extension.  User specifies this by setting the
   HPGL_ASSIGN_COLORS environment variable to "yes"; see above. */
static bool
#ifdef _HAVE_PROTOS
_parse_pen_string (const char *pen_s, Plotter *state_ptr)
#else
_parse_pen_string (pen_s, state_ptr)
     const char *pen_s;
     Plotter *state_ptr;
#endif
{
  const char *charp;
  char name[MAX_COLOR_NAME_LEN];
  int i;

  charp = pen_s;
  while (*charp)
    {
      int pen_num;
      bool got_digit;
      const char *tmp;
      const Colornameinfo *info;

      if (*charp == ':')	/* skip any ':' */
	{
	  charp++;
	  continue;		/* back to top of while loop */
	}
      pen_num = 0;
      got_digit = false;
      while (*charp >= '0' && *charp <= '9')
	{
	  pen_num = 10 * pen_num + (int)*charp - (int)'0';
	  got_digit = true;
	  charp++;
	}
      if (!got_digit || pen_num < 1 || pen_num >= MAX_NUM_PENS)
	return false;
      if (*charp != '=')
	return false;
      charp++;
      for (tmp = charp, i = 0; i < MAX_COLOR_NAME_LEN; tmp++, i++)
	{
	  if (*tmp == ':') /* end of color name string */
	    {
	      name[i] = '\0';
	      charp = tmp + 1;
	      break;
	    }
	  else if (*tmp == '\0') /* end of name string and env var also */
	    {
	      name[i] = '\0';
	      charp = tmp;
	      break;
	    }
	  else
	    name[i] = *tmp;
	}

      /* got color name string, parse it */
      if (_string_to_color (name, &info))
	{
	  state_ptr->pen_color[pen_num].red = info->red;
	  state_ptr->pen_color[pen_num].green = info->green;
	  state_ptr->pen_color[pen_num].blue = info->blue;
	  state_ptr->pen_defined[pen_num] = 2; /* hard-defined */
	}
      else			/* couldn't match color name string */
	return false;
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
_hpgl_terminate_plotter (Plotter *plotter)
#else
     _hpgl_terminate_plotter (plotter)
     Plotter *plotter;
#endif
{
    return true;
}
