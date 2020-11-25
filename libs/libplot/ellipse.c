/* This file contains the ellipse routine, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ellipse() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d %d\n", 
	     ELLIPSE, x, y, rx, ry, angle);
  else
    {
      putc (ELLIPSE, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (rx, _outstream);
      _putshort (ry, _outstream);
      _putshort (angle, _outstream);
    }
  
  return 0;
}

int
fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipse() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g\n", 
	     FELLIPSE, x, y, rx, ry, angle);
  else
    {
      putc (FELLIPSE, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
      _putfloat (rx, _outstream);
      _putfloat (ry, _outstream);
      _putfloat (angle, _outstream);
    }
  
  return 0;
}
