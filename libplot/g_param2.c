/* This database lists the Plotter parameters parameters (i.e. device
   driver parameters) that are recognized, and their default values.
   Whether or not a parameter is a string is listed too. */

/* The `default_value' field should be specified for each parameter that is
   string-valued (i.e. (char *)-valued).  [Exception: HPGL_PENS is set to
   NULL, because the default behavior is determined by the value of
   HPGL_VERSION.  See h_defplot.c.]. */

/* Beside each parameter there is a comment indicating which type(s) of
   Plotter the parameter is relevant to, and which datatype its value
   should be, if it is not a (char *). */

#include "sys-defines.h"
#include "extern.h"

const struct plParamRecord _known_params[NUM_PLOTTER_PARAMETERS] =
{
  /* String-valued (i.e. really (char *)-valued */

  {"AI_VERSION", (char *)"5", true}, /* ai [obsolescent; undocumented] */
  {"BG_COLOR", (char *)"white", true}, /* X, pnm, gif, cgm */
  {"BITMAPSIZE", (char *)"570x570", true}, /* X, pnm, gif */
  {"CGM_ENCODING", (char *)"binary", true}, /* cgm */
  {"CGM_MAX_VERSION", (char *)"4", true}, /* cgm */
  {"DISPLAY", (char *)"", true}, /* X */
  {"GIF_ANIMATION", (char *)"yes", true}, /* gif */
  {"GIF_DELAY", (char *)"0", true}, /* gif */
  {"GIF_ITERATIONS", (char *)"0", true}, /* gif */
  {"HPGL_ASSIGN_COLORS", (char *)"no", true}, /* hpgl */
  {"HPGL_OPAQUE_MODE", (char *)"yes", true}, /* hpgl */
  {"HPGL_PENS", (char *)NULL, true}, /* hpgl */
  {"HPGL_ROTATE", (char *)"no", true},	/* hpgl */
  {"HPGL_VERSION", (char *)"2", true},	/* hpgl */
  {"INTERLACE", (char *)"no", true}, /* gif */
  {"MAX_LINE_LENGTH", (char *)MAX_UNFILLED_POLYLINE_LENGTH_STRING, true}, /* all but tek and meta */
  {"META_PORTABLE", (char *)"no", true}, /* meta */
  {"PAGESIZE", (char *)"letter", true}, /* hpgl, pcl, fig, cgm, ps, ai */
  {"PCL_ASSIGN_COLORS", (char *)"no", true}, /* pcl */
  {"PCL_BEZIERS", (char *)"yes", true},	/* pcl */
  {"PNM_PORTABLE", (char *)"no", true}, /* pnm */
  {"ROTATION", (char *)"no", true}, /* tek, hpgl, pcl, fig, ps, ai, X, XDrawable */
  {"TERM", (char *)"tek", true}, /* tek only! */
  {"TRANSPARENT_COLOR", (char *)"none", true}, /* gif */
  {"USE_DOUBLE_BUFFERING", (char *)"no", true}, /* X, XDrawable */
  {"VANISH_ON_DELETE", (char *)"no", true}, /* X */
  {"X_AUTO_FLUSH", (char *)"yes", true}, /* X */

  /* Pointer-valued (i.e. non-string, i.e. non-(char *)-valued) */

  {"XDRAWABLE_COLORMAP", NULL, false}, /* XDrawable, is a Colormap* */
  {"XDRAWABLE_DISPLAY", NULL, false}, /* XDrawable, is a Display* */
  {"XDRAWABLE_DRAWABLE1", NULL, false}, /* XDrawable, is a Drawable* */
  {"XDRAWABLE_DRAWABLE2", NULL, false}, /* XDrawable, is a Drawable* */

  /* Spares */
  {"FOOBAR", (char *)"", true},
};
