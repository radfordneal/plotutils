/* This file contains the move method, which is a standard part of libplot.
   It sets a drawing attribute: the location of the graphics cursor, which
   determines the position of the next object drawn on the graphics
   device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fmove (double x, double y)
#else
_g_fmove (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmove: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  _plotter->drawstate->pos.x = x; /* update our notion of position */
  _plotter->drawstate->pos.y = y;

  return 0;
}
