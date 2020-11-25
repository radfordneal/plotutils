/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

/* In this generic version, we simply call the ellipse method. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fcircle (double x, double y, double r)
#else
_g_fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcircle: invalid operation");
      return -1;
    }

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing */
    {
      _plotter->endpath (); /* flush polyline if any */
      return 0;
    }

  return _plotter->fellipse (x, y, r, r, 0.0);
}
