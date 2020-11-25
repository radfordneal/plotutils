/* This is the master include file for the MI (machine independent)
   scan-conversion module, which is based on source files in the X Window
   System distribution.  The module consists of the files g_mi*.c and
   g_mi*.h.  For an X Consortium copyright notice, see the file README-X.
   GNU extensions copyright (c) 1998-99 by the Free Software Foundation.

   The MI module is essentially self-contained, with only a few drawing
   functions that serve as entry points (they are declared at the bottom of
   this file).  So in principle, you could remove the MI module from the
   libplot/libplotter distribution, and include it in other code.  The
   functions in the module draw into a bitmap (a 2-D array of miPixels).
   They take an `augmented graphics context' (a structure including such a
   bitmap, and a standard set of drawing parameters) as first argument.
   You would typedef miPixel to whatever is appropriate in your
   application, and also provide a definition of the samePixel() function,
   which tests two pixel values for equality.

   If you use the module in other code, you would need to include various
   system header files (e.g., <stdlib.h>, <limits.h> and <math.h>) for it
   to compile.  You would also need to delete the #include lines in each
   file that refer to "sys-defines.h" and "extern.h", which are specific to
   libplot/libplotter.  And you would need to define a few macros like
   ____P(), _HAVE_PROTOS, Voidptr, and IROUND(), but that is a trivial
   matter (the definitions for them are all in ../include/sys-defines.h).  */

/* structure that defines a point (e.g. the points that define a polyline
   or polygon) */
typedef struct
{
  int x, y;			/* integer coordinates, y goes downward */
} miIntPoint;

/* structure that defines a rectangle */
typedef struct
{
  int x, y;			/* upper left corner */
  unsigned int width, height;	/* pixel range */
} miRectangle;

/* structure that defines an `arc' (a segment of an ellipse aligned with
   the coordinate axes) */
typedef struct
{
  int x, y;			/* upper left corner of ellipse's bbox */
  unsigned int width, height;	/* dimensions of ellipse */
  int angle1, angle2;	/* starting angle and angle range, in 1/64 degrees */
} miArc;

/* Pixel value datatype.  Should be able to hold all pixel values.  In
   libplot/libplotter we use a union, so that this module can be used both
   by GIF Plotters (in which pixel values are 8-bit color indices) and by
   PNM Plotters (for which pixel values are 24-bit RGB values). */

/* In libplot/libplotter, the official definition of miPixel appears in
   plotter.h, so this definition is commented out.  Uncomment and modify
   appropriately if using this module in other code. */
#if 0
typedef union
{
  unsigned char index;		/* for GIF Plotters */
  unsigned char rgb[3];		/* for PNM Plotters */
} miPixel;
#endif

/* An `augmented graphics context', including a drawable (a 2-D array of
   pixels), and a graphics context (GC).  A pointer to one of these
   structures is passed as the first argument to each of the externally
   accessible drawing functions.

   The drawing functions treat the structure almost as a `const struct'.
   (The fgPixel and fillStyle members of the structure may be temporarily
   modified, but if so, they are restored; other members are not modified.)

   All fields should be filled in.  See g_miter.c for comments on the
   miter limit. */
typedef struct
{
  /* test of pixels for equality */
  bool (*samePixel) ____P((miPixel pixel1, miPixel pixel2));
  /* 2D array of miPixels; (0,0) is upper left corner */
  miPixel **drawable;
  unsigned int width;
  unsigned int height;
  /* GC entries */
  miPixel fgPixel;		/* color of pen, for most drawing operations */
  miPixel bgPixel;		/* used only in drawing double-dashed lines */
  int fillStyle;		/* see below */
  int fillRule;			/* see below */
  int joinStyle;		/* see below */
  int capStyle;			/* see below */
  int lineStyle;		/* see below */
  unsigned int lineWidth;	/* line thickness in pixels(for `wide lines')*/
  double miterLimit;		/* X Windows uses 10.43 for this */
  int arcMode;			/* for arc-drawing operations; see below */
  unsigned int *dash;		/* dash array (lengths of dashes in pixels) */
  int numInDashList;		/* length of dash array */
  int dashOffset;		/* pixel offset of first dash */
  /* for stippling and tiling */
  int **stipple;
  unsigned int stippleWidth;
  unsigned int stippleHeight;
  int stippleXorigin, stippleYorigin;
  miPixel **tile;
  unsigned int tileWidth;
  unsigned int tileHeight;
  int tileXorigin, tileYorigin;
} miGC;

/* values for fill style */
enum { miFillSolid, miFillTiled, miFillStippled, miFillOpaqueStippled };

/* values for fill rule */
enum { miEvenOddRule, miWindingRule };

/* values for join style */
enum { miJoinMiter, miJoinRound, miJoinBevel, miJoinTriangular };

/* values for cap style */
enum { miCapNotLast, miCapButt, miCapRound, miCapProjecting, miCapTriangular };

/* values for line style */
enum { miLineSolid, miLineOnOffDash, miLineDoubleDash };

/* values for arc mode */
enum { miArcChord, miArcPieSlice };

/*----------------------------------------------------------------------*/
/* Declarations of internal functions */

/* internal, low-level raster operation (all painting of the 2-D array of
   miPixels goes through this) */
extern void miFillSpans ____P((miGC *pGC, int n, const miIntPoint *ppt, const unsigned int *pwidth, bool sorted));

/* wrappers for storage allocation, see g_mialloc.c */
extern Voidptr mi_xcalloc ____P((unsigned int nmemb, unsigned int size));
extern Voidptr mi_xmalloc ____P((unsigned int size));
extern Voidptr mi_xrealloc ____P((Voidptr p, unsigned int size));

/* misc. internal functions */
extern void miFillConvexPoly ____P((miGC *pGC, int count, const miIntPoint *ptsIn));
extern void miFillGeneralPoly ____P((miGC *pGC, int count, const miIntPoint *ptsIn));
extern void miStepDash ____P((int dist, int *pDashIndex, const unsigned int *pDash, int numInDashList, int *pDashOffset));

/*----------------------------------------------------------------------*/

/* The drawing functions (i.e., entry points) supplied by the module. */

/* 1. Polyline-related drawing functions.

   The final three arguments of each are a `mode' (see below), a specified
   number of points, and an array of points, of the specified length.
   miFillPolygon draws a filled polygon, miPolyPoint draws a cloud of
   points, and miWideDash, miWideLine, miZeroDash, miZeroLine respectively
   draw a dashed wide polyline, an undashed (solid) wide polyline, a dashed
   zero-width polyline, and an undashed (solid) zero-width polyline.
   miWideDash and miWideLine simply invoke miZeroDash and miZeroLine if the
   line width in terms of pixels is zero.

   The final argument for miFillPolygon and miPolyPoint is an 
   (miIntPoint *), not a (const miIntPoint *), because if the mode is
   miCoordModePrevious, i.e. the points after the first are specified in
   relative coordinates, they rewrite the point array in absolute
   coordinates.  This should be fixed. */

extern void miFillPolygon ____P((miGC *pGC, int shape, int mode, int count, miIntPoint *pPts));
extern void miPolyPoint ____P((miGC *pGC, int mode, int npt, miIntPoint *pPts));
extern void miWideDash ____P((miGC *pGC, int mode, int npt, const miIntPoint *pPts));
extern void miWideLine ____P((miGC *pGC, int mode, int npt, const miIntPoint *pPts));
extern void miZeroDash ____P((miGC *pGC, int mode, int npt, const miIntPoint *pPts));
extern void miZeroLine ____P((miGC *pGC, int mode, int npt, const miIntPoint *pPts));

/* Values for the `mode' argument (are the points in the points array,
   after the first point, specified in absolute or relative
   coordinates?). */
enum { miCoordModeOrigin, miCoordModePrevious };

/* Values for the `shape' argument of miFillPolygon(): general (i.e., not
   necessarily convex, with self-intersections OK), or convex and not
   self-intersecting?  Latter case can be drawn more rapidly. */
enum { miComplex, miConvex };

/* 2. Rectangle-related drawing functions.

   miPolyFillRect fills a specified number of rectangles, supplied as an
   array of miRectangles. */

extern void miPolyFillRect ____P((miGC *pGC, int nrectFill, const miRectangle *prectInit));

/* 3. Arc-related drawing functions.

   Each of these takes as argument a multi-arc, i.e. a list of elliptic
   arcs.  Here, an elliptic arc is a piece of an ellipse whose axes are
   aligned with the coordinate axes (see above).  The arcs are not required
   to be contiguous.

   miPolyArc draws a wide multi-arc, which may be dashed.  If the arcs are
   contiguous, they will be joined in the usual way.  miZeroPolyArc draws a
   zero-width multi-arc, which may be dashed, using a fast algorithm.
   (miPolyArc does not automatically invoke miZeroPolyArc if the line width
   is zero.)  miPolyFillArc draws a sequence of filled arcs. */

extern void miPolyArc ____P((miGC *pGC, int narcs, const miArc *parcs));
extern void miPolyFillArc ____P((miGC *pGC, int narcs, const miArc *parcs));
extern void miZeroPolyArc ____P((miGC *pGC, int narcs, const miArc *parcs));
