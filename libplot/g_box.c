/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1.

   In this generic version, we simply call fmove() and fcont() repeatedly,
   to draw the box. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fbox (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
#else
_g_fbox (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1;
#endif
{
  double xnew, ynew;
  bool x_move_is_first;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbox: invalid operation");
      return -1;
    }

  /* move to starting vertex; invoke endpath() to flush out any path in
     progress */
  _plotter->fmove (R___(_plotter) x0, y0);

  /* pass hint to cont() and endpath() */
  _plotter->drawstate->convex_path = true;

  /* trace out rectangle in counterclockwise direction */
  x_move_is_first = ((x1 >= x0 && y1 >= y0)
		     || (x1 < x0 && y1 < y0) ? true : false);
  if (x_move_is_first)
    _plotter->fcont (R___(_plotter) x1, y0);
  else
    _plotter->fcont (R___(_plotter) x0, y1);
  _plotter->fcont (R___(_plotter) x1, y1);
  if (x_move_is_first)
    _plotter->fcont (R___(_plotter) x0, y1);
  else
    _plotter->fcont (R___(_plotter) x1, y0);
  _plotter->fcont (R___(_plotter) x0, y0);

  /* move to center (a libplot convention); this calls endpath() to flush
     out the polyline */
  xnew = 0.5 * (x0 + x1);
  ynew = 0.5 * (y0 + y1);
  _plotter->fmove (R___(_plotter) xnew, ynew);

  _plotter->drawstate->convex_path = false; /* restore hint value */

  return 0;
}
