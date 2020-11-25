/* This header file lists the device driver parameters (i.e. Plotter class
   variables) recognized by libplot.  It is #included by api.c, which
   provides the C binding to libplot. */

/* A default value must be specified for each parameter that is
   string-valued, except for HPGL_PENS, which is left uninitialized.  (Its
   default value will depend on the value of HPGL_VERSION; see h_openpl.c.)  */

struct param_record
{
  const char *parameter;
  const void *default_value;
  void *value;  
  bool is_string;
};

struct param_record _plot_params[NUM_DEVICE_DRIVER_PARAMETERS] =
{
  {"DISPLAY", NULL, NULL, true}, /* X */
  {"HPGL_ASSIGN_COLORS", "no", NULL, true}, /* hpgl */
  {"HPGL_OPAQUE_MODE", "no", NULL, true}, /* hpgl */
  {"HPGL_PENS", NULL, NULL, true}, /* hpgl, default is version-dependent */
  {"HPGL_VERSION", "2", NULL, true},	/* hpgl */
  {"HPGL_XOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"HPGL_YOFFSET", "0 cm", NULL, true}, /* hpgl */
  				/* all but tek and meta */
  {"MAX_LINE_LENGTH", MAX_UNFILLED_POLYLINE_LENGTH_STRING, NULL, true},
  {"META_PORTABLE", "no", NULL, true}, /* raw  */
  {"PAGESIZE", "letter", NULL, true}, /* hpgl, fig, ps */
  {"ROTATE", "no", NULL, true},	/* hpgl, fig, ps */
  {"TERM", "tek", NULL, true},	/* tek only! */
  {"VANISH_ON_DELETE", "no", NULL, true}, /* X */
  {"XDRAWABLE_DISPLAY", NULL, NULL, false}, /* XDrawable, is a Display* */
  {"XDRAWABLE_DRAWABLE1", NULL, NULL, false}, /* XDrawable, is a Drawable* */
  {"XDRAWABLE_DRAWABLE2", NULL, NULL, false}, /* XDrawable, is a Drawable* */
};
