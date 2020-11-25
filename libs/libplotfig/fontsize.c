/* This file contains the fontsize routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the size of the font used for
   text subsequently drawn on the graphics device.  The selected font may
   also be changed by calling the fontname or textangle routines.

   The argument to fontsize() is the size in user units.  The fontsize is
   normally taken to be a minimum acceptable vertical spacing between
   adjacent lines of text.  If a font size is not available, the closest
   available size will be substituted.

   The fontsize, fontname, and textangle routines return the fontsize in
   user units, as an aid to vertical positioning by the user.  The return
   value is zero if no information on font size is available. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
ffontsize (size)
     double size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ffontsize() called when graphics device not open\n");
      return -1;
    }

  if (size < 0.0)		/* reset to default */
    size = _default_drawstate.font_size;

  /* set the new nominal size in the drawing state */
  _drawstate->font_size = size;

  /* set fig_quantized_font_size, and (for PS fonts) fig_font_point_size */
  _set_font_sizes();

  /* return closest integer to quantized version of user-specified font size */
  return _drawstate->fig_quantized_font_size;
}

/* On paper, xfig supports arbitrary (non-integer) font sizes for PS fonts.
   But the current release rounds them to integers.  So we quantize the
   user-specified font size in such a way that the font size that fig will
   see, and use, will be precisely an integer. */
void
_set_font_sizes()
{
  double theta;
  double dx, dy, device_dx, device_dy, device_vector_len;
  double pointsize, fig_pointsize, size, quantized_size;
  int int_fig_pointsize;

  if (_drawstate->font_type == F_STROKE)
    {
      /* no quantization */
      _drawstate->fig_quantized_font_size = _drawstate->font_size;
      return;
    }
  
  /* text rotation in radians */
  theta = _drawstate->text_rotation * M_PI / 180.0;

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
      _drawstate->fig_font_point_size = 0;
      _drawstate->fig_quantized_font_size = 0.0;
      return;
    }

  /* xfig pointsize we should use when printing a string in a PS font, so
     as to match this vector length.  N.B. We match only the length, not
     the height, of the affinely transformed string.  That's because xfig
     doesn't currently support PS fonts that are scaled other than
     uniformly. */

  size = _drawstate->font_size; /* in user units */
  pointsize = FIG_UNITS_TO_POINTS(size * device_vector_len);

  /* FIG_FONT_SCALING = 80/72 is a silly undocumented factor that shouldn't
     exist, but does.  In xfig, a `point' is not 1/72 inch, but 1/80 inch!  */
  fig_pointsize = FIG_FONT_SCALING * pointsize;
  /* integer xfig pointsize (which really refers to ascent, not overall size)*/
  int_fig_pointsize = IROUND(fig_pointsize);
  
  /* integer font size that xfig will see, in the .fig file */
  _drawstate->fig_font_point_size = int_fig_pointsize;
  /* what size in user units should have been, to make fig_font_point_size
     an integer */
  quantized_size = 
    (POINTS_TO_FIG_UNITS((double)int_fig_pointsize / FIG_FONT_SCALING))
      / (device_vector_len);

  _drawstate->fig_quantized_font_size = quantized_size;

  return;
}
