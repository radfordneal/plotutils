/* Initialization for the first drawing state on the stack of drawing
   states maintained by any Plotter.  Its components include drawing
   attributes, and the state of any uncompleted path object.  (At
   initialization, there is none.) */
   
/* This is copied to the first state on the stack, in g_savestate.c.  The
   four fields, `font_name', `font_type', `typeface_index', and
   `font_index' are special: they are filled in at that time, since they
   are Plotter-dependent.  So the values for them below (respectively
   "HersheySerif", F_HERSHEY, 0, and 1) are really dummies. */

/* Two other fields (font size and line width in user coordinates) play an
   important role at later times, e.g. a bad font size resets the font size
   to the default.  For that reason, those variables are filled in when
   space() is called (see g_space.c).  They are computed using the two
   quantities DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE and
   DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE (defined in extern.h). */

#include "sys-defines.h"
#include "extern.h"

const plDrawState _default_drawstate = {

/***************** DEVICE-INDEPENDENT PART **************************/

/* affine transformation from user coordinates to device coordinates */
/* [DUMMY VALUES; filled in by initial call to space()] */
  {
    {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, /* PS-style transformation matrix */
    true,			/* transformation scaling is uniform? */
    true,			/* transf. preserves axis directions? */
    true,			/* transf. doesn't involve a reflection? */
  },
/* graphics cursor position */
  {0.0, 0.0},			/* cursor position, in user coordinates */
/* the state of any uncompleted path object */
  (plGeneralizedPoint *)NULL,	/* array of accumulated points [here NULL]*/
  0,				/* number of accumulated points */
  0,				/* length of point storage buffer (bytes) */
  false,			/* suppress endpath() while drawing a path? */
  false,			/* path to be drawn is convex? (endpath hint)*/
/* modal drawing attributes */
  /* 1. path-related attributes */
  "even-odd",			/* fill mode ["even-odd" / "nonzero-winding"]*/
  FILL_ODD_WINDING,		/* fill type, one of FILL_*, det'd by mode */
  "solid",			/* line mode [must be valid] */
  L_SOLID,			/* line type, one of L_*, det'd by line mode */
  true,				/* if not set, paths are "disconnected" */
  "butt",			/* cap mode [must be valid] */
  CAP_BUTT,			/* cap type, one of CAP_*, det'd by cap mode */
  "miter",			/* join mode [must be valid] */
  JOIN_MITER,			/* join type, one of JOIN_*, det'd by mode */
  DEFAULT_MITER_LIMIT,		/* miter limit for line joins */
  0.0,				/* line width in user coors [set by space()] */
  1.0,				/* line width in device coordinates ["] */
  1,				/* line width, quantized to integer ["] */
  (double *)NULL,		/* array of dash on/off lengths */
  0,				/* length of same */
  0.0,				/* offset distance into dash array (`phase') */
  false,			/* dash array should override line mode? */
  1,				/* pen type (0 = none, 1 = present) */
  0,				/* fill type (0 = none, 1 = present,...) */
  1,				/* orientation of circles etc.(1=c'clockwise)*/
  /* 2. text-related attributes */
  "HersheySerif",		/* font name [dummy, see g_savestate.c] */
  0.0,				/* font size in user coordinates [dummy] */
  0.0,				/* degrees counterclockwise, for labels */
  0.0,				/* true font size in user coordinates (") */  
  0.0,				/* font ascent in user coordinates (") */
  0.0,				/* font descent in user coordinates (") */
  F_HERSHEY,			/* font type [dummy] */
  0,				/* typeface index (in fontdb.h typeface table; this is Hershey Serif typeface) [dummy] */
  1,				/* font index (within typeface; this is Roman variant of Hershey Serif typeface) [dummy] */
  true,				/* true means an ISO-Latin-1 font ["] */
  /* color attributes (which affect both paths and text) */
  {0, 0, 0},			/* pen color (= black) */
  {0, 0, 0},			/* fill color (= black) */
  {65535, 65535, 65535},	/* background color (= white) */
/* Default values for certain modal attributes, used when an out-of-range
   value is requested. (These two are special because unlike all others,
   they're set by the initial call to space(), which also sets the line
   width and font size fields above.  Incidentally, space() also invokes
   linewidth(), so it will also set any device-dependent fields related to
   line width. */
  0.0,				/* line width in user coordinates */
  0.0,				/* font size in user coordinates */

/****************** DEVICE-DEPENDENT PART ***************************/

/* elements specific to the HP-GL drawing state [DUMMY, computed by space()] */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
/* elements specific to the Fig drawing state [DUMMIES] */
  16,				/* font size in fig's idea of points */
  -1,				/* fig's fill level (-1 = transparent) */
  C_BLACK,			/* fig's fg color (0=black, see colordb2.c) */
  C_BLACK,			/* fig's fill color (see list in colordb2.c) */
/* elements specific to the CGM Plotter drawing state */
  CGM_OBJECT_OTHER,		/* hint to set_attributes() (object to draw) */
/* elements specific to the PS drawing state [DUMMIES] */
  0.0,				/* RGB for PS fg color (floats) */
  0.0,
  0.0,
  1.0,				/* RGB for PS fill color (floats) */
  1.0,
  1.0,
  0,				/* idraw fg color (0=black, 9=white) */
  9,				/* idraw bg color (0=black, 9=white) */
  0,				/* shading (0=fg, 4=bg), if not transparent */
/* elements specific to the GIF drawing state [all save last 3 are DUMMIES] */
  {0, 0, 0},			/* 24-bit RGB of pixel for drawing (= black) */
  {0, 0, 0},			/* 24-bit RGB of pixel for filling (= black) */
  {255, 255, 255},		/* 24-bit RGB of pixel for erasing (= white) */
  (unsigned char)0,		/* drawing color index [dummy] */
  (unsigned char)0,		/* filling color index [dummy] */
  (unsigned char)0,		/* erasing color index [dummy] */
  false,			/* drawing color index is genuine? */
  false,			/* filling color index is genuine? */
  false,			/* erasing color index is genuine? */
#ifndef X_DISPLAY_MISSING
/* elements spec. to X11, X11 Drawable drawingstates [nearly all: DUMMY] */
  {14.0, 0.0, 0.0, 14.0},	/* pixel matrix, parsed from font name */
  true,				/* if set, can use XDrawString() etc. */
  (XFontStruct *)NULL,		/* font structure (used in x_alab_X.c) */
  (const unsigned char *)NULL,	/* label (hint to font retrieval routine) */
  (GC)NULL,			/* graphics context, for drawing */
  (GC)NULL,			/* graphics context, for filling */
  (GC)NULL,			/* graphics context, for erasing */
  X_GC_FOR_DRAWING,		/* hint to set_attributes() (GC to alter) */
  {0, 0, 0},			/* pen color stored in GC (= black) */
  {0, 0, 0},			/* fill color stored in GC (= black) */
  {65535, 65535, 65535},	/* bg color stored in GC (= white) */
  0,				/* fill level stored in GC (0 = transparent) */
  (unsigned long)0,		/* drawing pixel [dummy] */
  (unsigned long)0,		/* filling pixel [dummy] */
  (unsigned long)0,		/* erasing pixel [dummy] */
  false,			/* drawing pixel is genuine? */
  false,			/* filling pixel is genuine? */
  false,			/* erasing pixel is genuine? */
  LineSolid,			/* line style stored in drawing GC */
  CapButt,			/* cap style stored in drawing GC */
  JoinMiter,			/* join style stored in drawing GC */
  0,				/* line width stored in drawing GC */
  (char *)NULL,			/* dash list stored in drawing GC */
  0,				/* length of dash list stored in drawing GC */
  0,				/* offset into dash sequence, in drawing GC */
  EvenOddRule,			/* fill rule stored in filling GC */
#endif /* X_DISPLAY_MISSING */

/* pointer to previous drawing state */
  (plDrawState *)NULL		/* pointer to previous state [must be null] */
};
