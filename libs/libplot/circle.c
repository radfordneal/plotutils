/* This file contains the circle routine, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
circle (x, y, r)
     int x, y, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: circle() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d\n", 
	     CIRCLE, x, y, r);
  else
    {
      putc (CIRCLE, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (r, _outstream);
    }
  
  return 0;
}

int
fcircle (x, y, r)
     double x, y, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcircle() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g\n", 
	     FCIRCLE, x, y, r);
  else
    {
      putc (FCIRCLE, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
      _putfloat (r, _outstream);
    }
  
  return 0;
}
