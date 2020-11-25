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

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d\n", 
	     PENCOLOR, red, green, blue);
  else
    {
      putc (PENCOLOR, _outstream);
      _putshort (red, _outstream);
      _putshort (green, _outstream);
      _putshort (blue, _outstream);
    }
  
  return 0;
}
