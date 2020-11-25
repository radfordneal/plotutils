/* This file contains the line method, which is a standard part of libplot.
   It draws an object: a line segment extending from the point x0,y0 to the
   point x1,y1.  By repeatedly invoking cont(), the user may extend this
   line object to a polyline object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fline (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
#else
_g_fline (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fline: invalid operation");
      return -1;
    }

  /* If new line segment not contiguous, move to its starting point
     (thereby finalizing the path under construction, if any, since the
     move() method invokes the endpath() method). */
  if (x0 != _plotter->drawstate->pos.x
      || y0 != _plotter->drawstate->pos.y)
    _plotter->fmove (R___(_plotter) x0, y0);

  return _plotter->fcont (R___(_plotter) x1, y1);
}
