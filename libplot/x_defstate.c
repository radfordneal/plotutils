/* Initialization for an X drawing state, which is one of the data members
   of any XPlotter object.  Its components include drawing attributes, and
   the state of any uncompleted polyline object.  (At initialization, there
   is none.) */
   
/* Two state variables (font size and line width in user coordinates) play
   an important role at later times, e.g. a bad font size resets the font
   size to the default.  For that reason, those variables are filled in
   when space() is called (see g_space.c).  They are computed from the two
   quantities DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH and
   DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH (defined in
   extern.h). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

State _X_default_drawstate = {
/* affine transformation from user coordinates to device coordinates */
  {
    {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, /* PS-style transformation matrix */
    true,			/* transformation scaling is uniform? */
    true,			/* transf. preserves axis directions? */
    true,			/* transf. doesn't involve a reflection? */
    true, /* X is not NeWS */	/* raster device? */
  },
/* graphics cursor position */
  {0.0, 0.0},			/* graphics cursor position */
/* the state of any uncompleted polyline object */
  (Point *)NULL,		/* array of accumulated points [here NULL]*/
  0,				/* number of accumulated points */
  0,				/* length of point storage buffer (bytes) */
  false,			/* whether arc has been stored */
  false,			/* whether to output stored arc immediately */
  false,			/* whether to output as polyline (advisory) */
  0.0,0.0,0.0,0.0,0.0,0.0,	/* stored arc parameters */
/* modal drawing attributes */
  "solid",			/* line mode [must be valid] */
  L_SOLID,			/* line type, one of L_*, det'd by line mode */
  "miter",			/* join mode [must be valid] */
  JOIN_MITER,			/* join type, one of J_*, det'd by join mode */
  "butt",			/* cap mode [must be valid] */
  CAP_BUTT,			/* cap type, one of C_*, det'd by cap mode */
  0.0,				/* line width in user coors */
  0,				/* fill level (0 = transparent) */
  "Helvetica",			/* font name [must be valid] */
  0.0,				/* font size in user coordinates */
  0.0,				/* true font size in user coordinates (") */  
  0.0,				/* font ascent in user coordinates (") */
  0.0,				/* font descent in user coordinates (") */
  F_POSTSCRIPT,			/* font type */
  0,				/* typeface index (in fontdb.h typeface table; this is Helvetica)*/
  1,				/* font index (within typeface; this is the Helvetica font)*/
  true,				/* true means an ISO-Latin-1 font */
  0.0,				/* degrees counterclockwise, for labels */
  {0, 0, 0},			/* foreground color */
  {0, 0, 0},			/* fill color */
  true,				/* if not set, polyline reduces to points */
  1.0,				/* line width in device coordinates */
  1,				/* line width, quantized to integer */
  false,			/* suppress endpath() while drawing a path? */
/* elements specific to the HP-GL drawing state */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
/* elements specific to the fig drawing state */
  16,				/* font size in fig's idea of points */
  -1,				/* fig's fill level (-1 = transparent) */
  C_BLACK,			/* fig's fg color (0=black, see colordb2.c) */
  C_BLACK,			/* fig's fill color (see list in colordb2.c) */
/* elements specific to the PS drawing state */
  0.0,				/* RGB for PS fg color (floats) */
  0.0,
  0.0,
  1.0,				/* RGB for PS fill color (floats) */
  1.0,
  1.0,
  0,				/* idraw fg color (0=black, 9=white) */
  9,				/* idraw bg color (0=black, 9=white) */
  0,				/* shading (0=fg, 4=bg), if not transparent */
#ifndef X_DISPLAY_MISSING
/* elements specific to the X11 drawing state */
  {14.0, 0.0, 0.0, 14.0},	/* pixel matrix, parsed from font name */
  true,				/* if set, can use XDrawString() etc. */
  (GC)NULL,			/* graphics context, for drawing */
  (XFontStruct *)NULL,		/* font structure (used in alabel.c) */
  (unsigned long)0,		/* background pixel [dummy, set in openpl.c] */
  (unsigned long)0,		/* foreground pixel [dummy, set in openpl.c] */
  (unsigned long)0,		/* fill pixel [dummy, set elsewhere] */
#endif /* X_DISPLAY_MISSING */
/* pointer to previous drawing state */
  (State *)NULL			/* pointer to previous state [must be null] */
};
