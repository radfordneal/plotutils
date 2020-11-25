/* This file contains the arc routine, which is a standard part of libplot.
   It draws an object: a counterclockwise circular arc with the center at
   xc,yc, the beginning at x0,y0 and the ending at x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: arc() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d %d %d\n", 
	     ARC, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (ARC, _outstream);
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
farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: farc() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g %g\n", 
	     FARC, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (FARC, _outstream);
      _putfloat (xc, _outstream);
      _putfloat (yc, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }
  
  return 0;
}
