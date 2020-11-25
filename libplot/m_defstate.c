/* Initialization for a metafile drawing state, which is one of the data
   elements of any MetaPlotter object.  Its components include drawing
   attributes, and the state of any uncompleted polyline object.
   
   This is almost totally ignored by the metafile driver, as far as the
   libplot API goes.  The chief reason we allow a MetaPlotter object to
   maintain a stack of drawing states is to keep track of font sizes, so
   that the labelwidth() operation will work.  The font size fields below
   (font_size, true_font_size, also font_name of course) are initialized by
   the space() operation, and recomputed by the fontsize() and fontname()
   operations.

   We update all the other attribute fields too, but we don't use them.
   Also, we don't store points in constructed polylines in the drawing
   state; there's no point. :-) We just emit them to the graphics
   metafile. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

State _meta_default_drawstate = {
/* affine transformation from user coordinates to device coordinates */
  {
    {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, /* PS-style transformation matrix */
    true,			/* transformation scaling is uniform? */
    true,			/* transf. preserves axis directions? */
    true,			/* transf. doesn't involve a reflection? */
    true,			/* raster device? */
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
  0,				/* fill level */
  "HersheySerif",		/* font name [must be valid pointer] */
  0.0,				/* font size in user coordinates */
  0.0,				/* true font size in user coordinates (") */  
  0.0,				/* font ascent in user coordinates (") */
  0.0,				/* font descent in user coordinates (") */
  F_HERSHEY,			/* font type */
  0,				/* typeface index (in fontdb.h typeface table; this is Hershey Serif)*/
  1,				/* font index (within typeface; this is Roman) */
  true,				/* true means an ISO-Latin-1 font */
  0.0,				/* degrees counterclockwise, for labels */
  {0, 0, 0},			/* foreground color */
  {0, 0, 0},			/* fill color */
  true,				/* if not set, polyline reduces to points */
  1.0,				/* line width in device coordinates */
  1,				/* line width, quantized to integer */
  true,				/* suppress endpath() while drawing a path? */
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
