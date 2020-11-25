/* This file contains the fillcolor routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the fill color of objects created
   in the following drawing operations.  Actually the true fill color (if
   filling is not disabled) will be a desaturated version of the
   user-specified fill color.  The desaturation level is set by calling
   fill().

   In principle we support 48-bit color (16 bits, i.e. 0x0000 through
   0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fillcolor (red, green, blue)
     int red, green, blue;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fillcolor() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  (_drawstate->fillcolor).red = red;
  (_drawstate->fillcolor).green = green;  
  (_drawstate->fillcolor).blue = blue;

  /* For now, this has no effect; there's no tek_fillcolor.c yet. */

  return 0;
}
