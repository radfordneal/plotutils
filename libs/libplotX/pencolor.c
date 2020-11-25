/* This file contains the pencolor routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the pen color (``foreground
   color'') of objects created in the drawing operations that follow.  The
   fill color may be set separately, by calling fillcolor() and fill().

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
pencolor (red, green, blue)
     int red, green, blue;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: pencolor() called when graphics device not open\n");
      return -1;
    }
  
  endpoly();			/* flush polyline if any */
  
  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.fgcolor.red;
      green = _default_drawstate.fgcolor.green;
      blue = _default_drawstate.fgcolor.blue;
    }

  /* save our notion of foreground color */
  (_drawstate->fgcolor).red = red;
  (_drawstate->fgcolor).green = green;  
  (_drawstate->fgcolor).blue = blue;

  _drawstate->x_fgcolor_defined = FALSE;

  _handle_x_events();

  return 0;
}
