/* This file contains the line method, which is a standard part of libplot.
   It draws an object: a line segment extending from the point x0,y0 to the
   point x1,y1.  By repeatedly invoking cont(), the user may extend this
   line object to a polyline object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fline (double x0, double y0, double x1, double y1)
#else
_g_fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fline: invalid operation");
      return -1;
    }

  /* If new line segment not contiguous, move to its starting point
     (thereby finalizing the path under construction, if any, since the
     move() method invokes the endpath() method). */
  if (x0 != _plotter->drawstate->pos.x
      || y0 != _plotter->drawstate->pos.y)
    _plotter->fmove (x0, y0);

  return _plotter->fcont (x1, y1);
}
