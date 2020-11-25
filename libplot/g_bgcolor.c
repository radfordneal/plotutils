/* This file contains the bgcolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the background color.

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_bgcolor (R___(Plotter *_plotter) int red, int green, int blue)
#else
_g_bgcolor (R___(_plotter) red, green, blue)
     S___(Plotter *_plotter;) 
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "bgcolor: invalid operation");
      return -1;
    }

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.bgcolor.red;
      green = _default_drawstate.bgcolor.green;
      blue = _default_drawstate.bgcolor.blue;
    }

  /* save our notion of background color */
  _plotter->drawstate->bgcolor.red = red;
  _plotter->drawstate->bgcolor.green = green;  
  _plotter->drawstate->bgcolor.blue = blue;

  return 0;
}
