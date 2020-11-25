/* This header file is #included by g_params.c.  It is a database rather
   than a true header file: it lists the device driver parameters
   (i.e. Plotter class variables) that are recognized. */

/* The `default_value' field should be specified for each parameter that is
   string-valued (i.e. (char *)-valued).  [Exception: HPGL_PENS is set to
   NULL, because the default behavior is determined by the value of
   HPGL_VERSION.  See h_defplot.c.]  The `value' field should always be
   NULL; it is set with the parampl() function. */

/* Beside each parameter there is a comment indicating which type(s) of
   Plotter the parameter is relevant to, and which datatype its value
   should be, if it is not a (char *). */

struct param_record
{
  const char *parameter;	/* parameter name */
  Voidptr default_value;	/* default value (applies if string-valued) */
  Voidptr value;		/* value (set by parampl() function) */
  bool is_string;		/* whether or not value must be a string */
};

static struct param_record _plot_params[NUM_PLOTTER_PARAMETERS] =
{
  /* String-valued (i.e. (char *)-valued */

  {"AI_VERSION", "5", NULL, true}, /* ai */
  {"BG_COLOR", "white", NULL, true}, /* X, pnm, gif */
  {"BITMAPSIZE", "570x570", NULL, true}, /* X, pnm, gif */
  {"DISPLAY", "", NULL, true}, /* X, XDrawable */
  {"GIF_ANIMATION", "yes", NULL, true}, /* gif */
  {"GIF_DELAY", "0", NULL, true}, /* gif */
  {"GIF_ITERATIONS", "0", NULL, true}, /* gif */
  {"HPGL_ASSIGN_COLORS", "no", NULL, true}, /* hpgl */
  {"HPGL_OPAQUE_MODE", "yes", NULL, true}, /* hpgl */
  {"HPGL_PENS", NULL, NULL, true}, /* hpgl */
  {"HPGL_ROTATE", "no", NULL, true},	/* hpgl */
  {"HPGL_VERSION", "2", NULL, true},	/* hpgl */
  {"HPGL_XOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"HPGL_YOFFSET", "0 cm", NULL, true}, /* hpgl */
  {"INTERLACE", "no", NULL, true}, /* gif */
  {"MAX_LINE_LENGTH", MAX_UNFILLED_POLYLINE_LENGTH_STRING, NULL, true}, /* all but tek and meta */
  {"META_PORTABLE", "no", NULL, true}, /* meta */
  {"PAGESIZE", "letter", NULL, true}, /* hpgl, pcl, fig, ps, ai */
  {"PCL_ASSIGN_COLORS", "no", NULL, true}, /* pcl */
  {"PCL_BEZIERS", "yes", NULL, true},	/* pcl */
  {"PCL_ROTATE", "no", NULL, true},	/* pcl */
  {"PCL_XOFFSET", "0 cm", NULL, true}, /* pcl */
  {"PCL_YOFFSET", "0 cm", NULL, true}, /* pcl */
  {"PNM_PORTABLE", "no", NULL, true}, /* pnm */
  {"TERM", "tek", NULL, true},	/* tek only! */
  {"TRANSPARENT_COLOR", "none", NULL, true}, /* gif */
  {"USE_DOUBLE_BUFFERING", "no", NULL, true}, /* X */
  {"VANISH_ON_DELETE", "no", NULL, true}, /* X */

  /* Pointer-valued (i.e. non-string, i.e. non-(char *)-valued) */

  {"XDRAWABLE_COLORMAP", NULL, NULL, false}, /* XDrawable, is a Colormap* */
  {"XDRAWABLE_DISPLAY", NULL, NULL, false}, /* XDrawable, is a Display* */
  {"XDRAWABLE_DRAWABLE1", NULL, NULL, false}, /* XDrawable, is a Drawable* */
  {"XDRAWABLE_DRAWABLE2", NULL, NULL, false}, /* XDrawable, is a Drawable* */
};
