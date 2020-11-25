/* This file contains the fillcolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the fill color of objects created
   in the following drawing operations.  Actually the true fill color (if
   filling is not disabled) will be a desaturated version of the
   user-specified fill color.  The desaturation level is set by invoking
   filltype().

   In principle we support 48-bit color (16 bits, i.e. 0x0000 through
   0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fillcolor (int red, int green, int blue)
#else
_g_fillcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fillcolor: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.fillcolor.red;
      green = _default_drawstate.fillcolor.green;
      blue = _default_drawstate.fillcolor.blue;
    }

  /* save our notion of fillcolor */
  _plotter->drawstate->fillcolor.red = red;
  _plotter->drawstate->fillcolor.green = green;  
  _plotter->drawstate->fillcolor.blue = blue;

  return 0;
}
