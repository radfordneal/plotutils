/* This file contains the ellarcrel() routine, which is a GNU extension to
   libplot.  ellarcrel() is simply a version of ellarc() that uses relative
   coordinates.  It draws an object: a counterclockwise arc with specified
   center, beginning, and end. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
ellarcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ellarcrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d %d %d\n", 
	     ELLARCREL, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (ELLARCREL, _outstream);
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
fellarcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellarcrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g %g\n", 
	     FELLARCREL, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (FELLARCREL, _outstream);
      _putfloat (xc, _outstream);
      _putfloat (yc, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }

  return 0;
}
