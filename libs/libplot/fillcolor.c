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

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d\n", 
	     FILLCOLOR, red, green, blue);
  else
    {
      putc (FILLCOLOR, _outstream);
      _putshort (red, _outstream);
      _putshort (green, _outstream);
      _putshort (blue, _outstream);
    }

  return 0;
}
