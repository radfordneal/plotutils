/* This file contains the move routine, which is a standard part of
   libplot.  It sets a drawing attribute: the location of the graphics
   cursor, which determines the position of the next object drawn on the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
move (x, y)
     int x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: move() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d\n", 
	     MOVE, x, y);
  else
    {
      putc (MOVE, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
    }
  
  return 0;
}

int
fmove (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmove() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n", 
	     FMOVE, x, y);
  else
    {
      putc (FMOVE, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }
  
  return 0;
}
