/* This file contains the point routine, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
point (x, y)
     int x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: point() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d\n", 
	     POINT, x, y);
  else
    {
      putc (POINT, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
    }
      
  return 0;
}

int
fpoint (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fpoint() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n", 
	     FPOINT, x, y);
  else
    {
      putc (FPOINT, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }
      
  return 0;
}
