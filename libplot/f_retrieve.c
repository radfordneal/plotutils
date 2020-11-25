/* This file contains the internal _retrieve_font method, which is called
   when the font_name, font_size, and textangle fields of the current
   drawing state have been filled in.  It retrieves the specified font, and
   fills in the font_type, typeface_index, font_index, font_is_iso8858,
   true_font_size, and font_ascent, and font_descent fields of the drawing
   state. */

/* This fig-specific version is needed in large part because xfig supports
   arbitrary (non-integer) font sizes for PS fonts only on paper.  The
   current releases (3.1 and 3.2) of xfig round them to integers.  So we
   quantize the user-specified font size in such a way that the font size
   that fig will see, and use, will be precisely an integer.  We store this
   in the fig_font_point_size field in the current drawing state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_f_retrieve_font(void)
#else
_f_retrieve_font()
#endif
{
  double theta;
  double dx, dy, device_dx, device_dy, device_vector_len;
  double pointsize, fig_pointsize, size, quantized_size;
  int int_fig_pointsize;

  /* invoke generic method */
  _g_retrieve_font();

  if (_plotter->drawstate->font_type != F_POSTSCRIPT)
    /* must be Hershey font, no additional quantization */
    return;
  
  if (!_plotter->drawstate->transform.uniform 
      || !_plotter->drawstate->transform.nonreflection)
    /* anamorphically transformed PS font not supported, use Hershey */
    {
      char *user_specified_name;

      user_specified_name = _plotter->drawstate->font_name;
      _plotter->drawstate->font_name = FIG_DEFAULT_HERSHEY_FONT;
      _f_retrieve_font();	/* recursive call */
      _plotter->drawstate->font_name = user_specified_name;
      return;

#if 0
      /* squawk */
      if (!_plotter->font_warning_issued)
	{
	  char *buf;
	  
	  buf = (char *)_plot_xmalloc (strlen (_plotter->drawstate->font_name) + strlen (FIG_DEFAULT_HERSHEY_FONT) + 100);
	  sprintf (buf, "cannot retrieve font \"%s\", using default \"%s\"", 
		   _plotter->drawstate->font_name, 
		   FIG_DEFAULT_HERSHEY_FONT);
	  _plotter->warning (buf);
	  free (buf);
	  _plotter->font_warning_issued = true;
	}
#endif
    }

  /* text rotation in radians */
  theta = _plotter->drawstate->text_rotation * M_PI / 180.0;

  /* unit vector along which we'll move when printing label */
  dx = cos (theta);
  dy = sin (theta);

  /* convert to device frame, and compute length in fig units */
  device_dx = XDV(dx, dy);
  device_dy = YDV(dx, dy);  
  device_vector_len = sqrt(device_dx * device_dx + device_dy * device_dy);

  /* if zero, bail out right now to avoid an FPE */
  if (device_vector_len == 0.0)
    {
      _plotter->drawstate->fig_font_point_size = 0;
      _plotter->drawstate->true_font_size = 0.0;
      return;
    }

  /* compute xfig pointsize we should use when printing a string in a PS
     font, so as to match this vector length. */

  size = _plotter->drawstate->font_size; /* in user units */
  pointsize = FIG_UNITS_TO_POINTS(size * device_vector_len);

  /* FIG_FONT_SCALING = 80/72 is a silly undocumented factor that shouldn't
     exist, but does.  In xfig, a `point' is not 1/72 inch, but 1/80 inch!  */
  fig_pointsize = FIG_FONT_SCALING * pointsize;
  /* integer xfig pointsize (which really refers to ascent, not overall size)*/
  int_fig_pointsize = IROUND(fig_pointsize);
  
  /* integer font size that xfig will see, in the .fig file */
  _plotter->drawstate->fig_font_point_size = int_fig_pointsize;
  /* what size in user units should have been, to make fig_font_point_size
     an integer */
  quantized_size = 
    (POINTS_TO_FIG_UNITS((double)int_fig_pointsize / FIG_FONT_SCALING))
      / (device_vector_len);

  /* quantize the three relevant fields */
  _plotter->drawstate->true_font_size = quantized_size;
  _plotter->drawstate->font_ascent *= (quantized_size /_plotter->drawstate->font_size);
  _plotter->drawstate->font_descent *= (quantized_size /_plotter->drawstate->font_size);  

  return;
}
