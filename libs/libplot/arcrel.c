/* This file contains the arcrel() routine, which is a GNU extension to
   libplot.  arcrel() is simply a version of arc() that uses relative
   coordinates.  It draws an object: a counterclockwise arc with specified
   center, beginning, and end. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
arcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: arcrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d %d %d\n", 
	     ARCREL, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (ARCREL, _outstream);
      _putshort (xc, _outstream);
      _putshort (yc, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
    }

  return 0;
}

int
farcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: farcrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g %g\n", 
	     FARCREL, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (FARCREL, _outstream);
      _putfloat (xc, _outstream);
      _putfloat (yc, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }

  return 0;
}
