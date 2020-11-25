/* This file contains the box routine, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: box() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d\n", 
	     BOX, x0, y0, x1, y1);
  else
    {
      putc (BOX, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
    }

  return 0;
}

int
fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fbox() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g\n", 
	     FBOX, x0, y0, x1, y1);
  else
    {
      putc (FBOX, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }

  return 0;
}
