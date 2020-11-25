/* This header file lists the device driver parameters (i.e. Plotter class
   variables) recognized by libplot.  It is #included by api.c, which
   provides the C binding to libplot. */

/* A default value must be specified for each parameter that is
   string-valued, except for HPGL_PENS, which is left uninitialized.  (Its
   default value will depend on the value of HPGL_VERSION; see h_openpl.c.)  */

struct param_record
{
  const char * const parameter;
  const Voidptr default_value;
  Voidptr value;  
  const bool is_string;
};

struct param_record _plot_params[NUM_DEVICE_DRIVER_PARAMETERS] =
{
  {"BG_COLOR", NULL, NULL, true}, /* X */
  {"BITMAPSIZE", NULL, NULL, true}, /* X */
  {"DISPLAY", NULL, NULL, true}, /* X, XDrawable */
  {"HPGL_ASSIGN_COLORS", "no", NULL, true}, /* hpgl */
  {"HPGL_OPAQUE_MODE", "no", NULL, true}, /* hpgl */
  {"HPGL_PENS", NULL, NULL, true}, /* hpgl, default is version-dependent */
  {"HPGL_VERSION", "2", NULL, true},	/* hpgl */
  {"HPGL_XOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"HPGL_YOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"MAX_LINE_LENGTH", MAX_UNFILLED_POLYLINE_LENGTH_STRING, NULL, true}, /* all but tek and meta */
  {"META_PORTABLE", "no", NULL, true}, /* raw  */
  {"PAGESIZE", "letter", NULL, true}, /* hpgl, fig, ps */
  {"ROTATE", "no", NULL, true},	/* hpgl, fig, ps */
  {"TERM", "tek", NULL, true},	/* tek only! */
  {"USE_DOUBLE_BUFFERING", "no", NULL, true}, /* X */
  {"VANISH_ON_DELETE", "no", NULL, true}, /* X */
  {"XDRAWABLE_COLORMAP", NULL, NULL, false}, /* XDrawable, is a Colormap* */
  {"XDRAWABLE_DISPLAY", NULL, NULL, false}, /* XDrawable, is a Display* */
  {"XDRAWABLE_DRAWABLE1", NULL, NULL, false}, /* XDrawable, is a Drawable* */
  {"XDRAWABLE_DRAWABLE2", NULL, NULL, false}, /* XDrawable, is a Drawable* */
};
