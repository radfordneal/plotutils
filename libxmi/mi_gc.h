/* A miGC structure contains high-level drawing parameters.  Such
   structures are created, modified, and destroyed by functions in mi_gc.c.
   miGC is typedef'd as lib_miGC in xmi.h.  lib_miGC is defined here,
   privately, so that an miGC will be opaque.

   Significant differences from X11 are (1) the dash array is an array of
   unsigned int's rather than char's, so that much longer dashes may be
   drawn, and (2) the miter limit is a GC attribute like any other (in X11,
   it is fixed at 10.43, and may not be altered). */

struct lib_miGC
{
  /* high-level drawing parameters not related to dashing */
  int fillRule;			/* default = miEvenOddRule */
  int joinStyle;		/* default = miJoinMiter */
  int capStyle;			/* default = miCapButt */
  int lineStyle;		/* default = miLineSolid */
  int arcMode;			/* default = miArcPieSlice */
  unsigned int lineWidth;	/* line thickness in pixels (default = 0) */
  double miterLimit;		/* default = 10.43, as in X11 */

  /* pixel types */
  miPixel *pixels;		/* array of pixel types */
  int numPixels;		/* number of pixel types (must be >=2) */

  /* high-level drawing parameters related to dashing */
  unsigned int *dash;		/* dash array (lengths of dashes in pixels) */
  int numInDashList;		/* length of dash array */
  int dashOffset;		/* pixel offset of first dash (nonnegative) */
};
