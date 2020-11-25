/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_fpoint (R___(Plotter *_plotter) double x, double y)
#else
_API_fpoint (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fpoint: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  if (_plotter->drawstate->pen_type == 0)
    /* no pen to draw with, so do nothing */
    return 0;

  /* call internal function: draw marker at current location, in a
     Plotter-specific way */
  _plotter->paint_point (S___(_plotter));

  return 0;
}

/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

/* This implementation is for a generic Plotter.  It does nothing. */

void
#ifdef _HAVE_PROTOS
_g_paint_point (S___(Plotter *_plotter))
#else
_g_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}
