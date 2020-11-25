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
_g_flinewidth(double new_line_width)
#else
_g_flinewidth(new_line_width)
     double new_line_width;
#endif
{
  double device_line_width, min_sing_val, max_sing_val;
  int quantized_device_line_width;

  if (!_plotter->open)
    {
      _plotter->error ("flinewidth: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  if (new_line_width < 0.0)	/* reset to default */
    new_line_width = _plotter->drawstate->default_line_width;

  /* set the new linewidth in the drawing state */
  _plotter->drawstate->line_width = new_line_width;
  
  /* also compute and set the device-frame line width */

  _matrix_sing_vals (_plotter->drawstate->transform.m, 
		     &min_sing_val, &max_sing_val);
  device_line_width = min_sing_val * new_line_width;

  /* don't use 0-width lines if user specified nonzero width */
  quantized_device_line_width = IROUND(device_line_width);
  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;
  
  _plotter->drawstate->device_line_width = device_line_width;
  _plotter->drawstate->quantized_device_line_width 
    = quantized_device_line_width;

  return 0;
}
