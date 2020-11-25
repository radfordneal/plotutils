/* This file contains the line routine, which is a standard part of
   libplot.  It draws an object: a line segment extending from the point
   x0,y0 to the point x1,y1.  By repeatedly calling cont(), the user may
   extend this line object to a polyline object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: line() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf(_outstream, "%c %d %d %d %d\n", 
	    LINE, x0, y0, x1, y1);
  else
    {
      putc (LINE, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
    }

  return 0;
}

int
fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fline() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf(_outstream, "%c %g %g %g %g\n", 
	    FLINE, x0, y0, x1, y1);
  else
    {
      putc (FLINE, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }

  return 0;
}
