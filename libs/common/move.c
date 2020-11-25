/* This file contains the move routine, which is a standard part of
   libplot.  It sets a drawing attribute: the location of the graphics
   cursor, which determines the position of the next object drawn on the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fmove (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmove() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  (_drawstate->pos).x = x;	/* update our notion of position */
  (_drawstate->pos).y = y;

  return 0;
}

int
move (x,y)
     int x,y;
{
  return fmove ((double)x, (double)y);
}
