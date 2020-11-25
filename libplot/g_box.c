/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1.

   In this generic version, we simply call fmove() and fcont() repeatedly,
   to draw the box. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fbox (double x0, double y0, double x1, double y1)
#else
_g_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  double xnew, ynew;

  if (!_plotter->open)
    {
      _plotter->error ("fbox: invalid operation");
      return -1;
    }

  /* move to starting vertex; invoke endpath() to flush out any path in
     progress */
  _plotter->fmove (x0, y0);

  _plotter->fcont (x0, y1);
  _plotter->fcont (x1, y1);
  _plotter->fcont (x1, y0);
  _plotter->fcont (x0, y0);

  /* move to center (libplot convention); this will invoke endpath() */
  xnew = 0.5 * (x0 + x1);
  ynew = 0.5 * (y0 + y1);
  _plotter->drawstate->convex_path = true; /* pass hint to endpath() */
  _plotter->fmove (xnew, ynew);
  _plotter->drawstate->convex_path = false; /* restore hint value */

  return 0;
}
