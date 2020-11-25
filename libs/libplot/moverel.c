/* This file contains the moverel() routine, which is a GNU extension to
   libplot.  moverel() is simply a version of move() that uses relative
   coordinates.  It sets a drawing attribute: the location of the graphics
   cursor, which determines the position of the next object drawn on the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
moverel (x, y)
     int x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: moverel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d\n", 
	     MOVEREL, x, y);
  else
    {
      putc (MOVEREL, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
    }
  
  return 0;
}

int
fmoverel (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmoverel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n", 
	     FMOVEREL, x, y);
  else
    {
      putc (FMOVEREL, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }
  
  return 0;
}
