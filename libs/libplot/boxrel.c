/* This file contains the boxrel routine, which is a GNU extension to
   libplot.  boxrel() is simply a version of box() that uses relative
   coordinates.  It draws an object: an upright rectangle with specified
   diagonal corners. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
boxrel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: boxrel() called when graphics device not open\n");
      return -1;
    }
  
  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d\n", 
	     BOXREL, x0, y0, x1, y1);
  else
    {
      putc (BOXREL, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
    }

return 0;
}

int
fboxrel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fboxrel() called when graphics device not open\n");
      return -1;
    }
  
  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g\n", 
	     FBOXREL, x0, y0, x1, y1);
  else
    {
      putc (FBOXREL, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }

return 0;
}
