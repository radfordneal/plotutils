/* This file contains the linerel() routine, which is a GNU extension to
   libplot.  linerel() is simply a version of line() that uses relative
   coordinates.  It draws an object: a line segment extending from one
   specified point to another.  By repeatedly calling cont(), the user may
   extend this line object to a polyline object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
linerel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linerel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf(_outstream, "%c %d %d %d %d\n", 
	    LINEREL, x0, y0, x1, y1);
  else
    {
      putc (LINEREL, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
    }

  return 0;
}

int
flinerel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: flinerel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf(_outstream, "%c %g %g %g %g\n", 
	    FLINEREL, x0, y0, x1, y1);
  else
    {
      putc (FLINEREL, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }

  return 0;
}
