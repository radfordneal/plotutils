/* This file contains the moverel() routine, which is a GNU extension to
   libplot.  moverel() is simply a version of move() that uses relative
   coordinates.  It sets a drawing attribute: the location of the graphics
   cursor, which determines the position of the next object drawn on the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fmoverel (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmoverel() called when graphics device not open\n");
      return -1;
    }

  fmove ((_drawstate->pos).x + x, (_drawstate->pos).y + y);
  
  return 0;
}

int
moverel (x, y)
     int x, y;
{
  return fmoverel ((double)x, (double)y);
}

