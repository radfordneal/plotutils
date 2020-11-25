/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units.

   It also computes an estimate for the width of lines in device units.
   This quantity is used by display devices that do not support `sheared
   lines'. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_flinewidth(R___(Plotter *_plotter) double new_line_width)
#else
_g_flinewidth(R___(_plotter) new_line_width)
     S___(Plotter *_plotter;)
     double new_line_width;
#endif
{
  double device_line_width, min_sing_val, max_sing_val;
  int quantized_device_line_width;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "flinewidth: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (new_line_width < 0.0)	/* reset to default */
    new_line_width = _plotter->drawstate->default_line_width;

  /* set the new linewidth in the drawing state */
  _plotter->drawstate->line_width = new_line_width;
  
  /* Also compute and set the device-frame line width, and a quantized
     (i.e. integer) version of same, which is used by most Plotters that
     use integer device coordinates. */

  _matrix_sing_vals (_plotter->drawstate->transform.m, 
		     &min_sing_val, &max_sing_val);
  device_line_width = min_sing_val * new_line_width;
  quantized_device_line_width = IROUND(device_line_width);

  /* Don't quantize the device-frame line width to 0 if user specified
     nonzero width.  If it has a bitmap display (rendered with libxmi),
     quantizing to 0 might be regarded as OK, since libxmi treats 0-width
     lines as Bresenham lines rather than invisible.  However, the Hershey
     fonts don't look good at small sizes if their line segments are
     rendered as Bresenham lines.  */

  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;
  
  _plotter->drawstate->device_line_width = device_line_width;
  _plotter->drawstate->quantized_device_line_width 
    = quantized_device_line_width;

  return 0;
}
