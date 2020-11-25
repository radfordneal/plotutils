/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

/* This version may be applied only to FigPlotter objects, since its
   version of the internal routine _device_line_width is customized for
   xfig.  xfig expresses line widths in terms of `Fig display units' rather
   than `Fig units'. */

#include "sys-defines.h"
#include "extern.h"

int 
#ifdef _HAVE_PROTOS
_f_flinewidth (R___(Plotter *_plotter) double new_line_width)
#else
_f_flinewidth (R___(_plotter) new_line_width)
     S___(Plotter *_plotter;)
     double new_line_width;
#endif
{
  double device_line_width;
  int quantized_device_line_width;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "flinewidth: invalid operation");
      return -1;
    }

  /* invoke generic method */
  _g_flinewidth (R___(_plotter) new_line_width);

  /* xfig expresses line widths in terms of `Fig display units', so we
     scale appropriately */
  device_line_width =
    FIG_UNITS_TO_FIG_DISPLAY_UNITS (_plotter->drawstate->device_line_width);
  /* don't use 0-width lines if user specified nonzero width */
  quantized_device_line_width = IROUND(device_line_width);
  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;
  
  _plotter->drawstate->device_line_width = device_line_width;
  _plotter->drawstate->quantized_device_line_width 
    = quantized_device_line_width;

  return 0;
}
