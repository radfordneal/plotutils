/* This file contains the pencolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the pen color (``foreground
   color'') of objects created in the drawing operations that follow.  The
   fill color may be set separately, by invoking fillcolor() and
   filltype().

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_pencolor (int red, int green, int blue)
#else
_g_pencolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("pencolor: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.fgcolor.red;
      green = _default_drawstate.fgcolor.green;
      blue = _default_drawstate.fgcolor.blue;
    }

  /* save our notion of foreground color */
  _plotter->drawstate->fgcolor.red = red;
  _plotter->drawstate->fgcolor.green = green;  
  _plotter->drawstate->fgcolor.blue = blue;

  return 0;
}
