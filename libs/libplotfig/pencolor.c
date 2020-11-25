/* This file contains the pencolor routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the pen color (``foreground
   color'') of objects created in the drawing operations that follow.  The
   fill color may be set separately, by calling fillcolor() and fill().

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue).

   In principle we support 48-bit color (16 bits, i.e. 0x0000 through
   0xffff, for each of red, green, and blue).  However, xfig supports only
   24-bit color (8 bits for each primary).  xfig also supports a non-RGB
   `default' color, which we use as the initial value for the color.  To
   switch to it at some later time, the user may call pencolor() with
   out-of-bounds arguments.  See colordb.c. */

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

      _drawstate->fig_fgcolor = _default_drawstate.fig_fgcolor;
      _drawstate->fig_fgcolor_defined = TRUE;
    }
  else
    _drawstate->fig_fgcolor_defined = FALSE; /* lazily evaluated */

  (_drawstate->fgcolor).red = red;
  (_drawstate->fgcolor).green = green;  
  (_drawstate->fgcolor).blue = blue;
  
  return 0;
}

