/* This header file is #included by api.c, which provides the C binding to
   libplot.  It is a database rather than a true header file: it lists the
   device driver parameters (i.e. Plotter class variables) recognized by
   libplot. */

/* A default value should be specified for each parameter that is
   string-valued, except for HPGL_PENS, which is left uninitialized.  (Its
   default value will depend on the value of HPGL_VERSION; see
   h_defplot.c.).  The `value' field should always be NULL; it is set with
   the parampl() function. */

/* Beside each parameter there is an indication of which type(s) of Plotter
   the parameter is relevant to, and which datatype its value should be, if
   it is not a string. */

struct param_record
{
  const char *parameter;	/* parameter name */
  Voidptr default_value;	/* default value (applies if string-valued) */
  Voidptr value;		/* value (set by parampl() function) */
  bool is_string;		/* whether or not value must be a string */
};

static struct param_record _plot_params[NUM_DEVICE_DRIVER_PARAMETERS] =
{
  {"BG_COLOR", NULL, NULL, true}, /* X */
  {"BITMAPSIZE", NULL, NULL, true}, /* X */
  {"DISPLAY", NULL, NULL, true}, /* X, XDrawable */
  {"HPGL_ASSIGN_COLORS", "no", NULL, true}, /* hpgl */
  {"HPGL_OPAQUE_MODE", "yes", NULL, true}, /* hpgl */
  {"HPGL_PENS", NULL, NULL, true}, /* hpgl, default is version-dependent */
  {"HPGL_ROTATE", "no", NULL, true},	/* hpgl */
  {"HPGL_VERSION", "2", NULL, true},	/* hpgl */
  {"HPGL_XOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"HPGL_YOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"MAX_LINE_LENGTH", MAX_UNFILLED_POLYLINE_LENGTH_STRING, NULL, true}, /* all but tek and meta */
  {"META_PORTABLE", "no", NULL, true}, /* raw  */
  {"PAGESIZE", "letter", NULL, true}, /* hpgl, pcl, fig, ps */
  {"PCL_ASSIGN_COLORS", "no", NULL, true}, /* pcl */
  {"PCL_ROTATE", "no", NULL, true},	/* pcl */
  {"PCL_XOFFSET", "0 cm", NULL, true}, /* pcl */
  {"PCL_YOFFSET", "0 cm", NULL, true}, /* pcl */
  {"TERM", "tek", NULL, true},	/* tek only! */
  {"USE_DOUBLE_BUFFERING", "no", NULL, true}, /* X */
  {"VANISH_ON_DELETE", "no", NULL, true}, /* X */
  {"XDRAWABLE_COLORMAP", NULL, NULL, false}, /* XDrawable, is a Colormap* */
  {"XDRAWABLE_DISPLAY", NULL, NULL, false}, /* XDrawable, is a Display* */
  {"XDRAWABLE_DRAWABLE1", NULL, NULL, false}, /* XDrawable, is a Drawable* */
  {"XDRAWABLE_DRAWABLE2", NULL, NULL, false}, /* XDrawable, is a Drawable* */
};
