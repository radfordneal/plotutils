/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

/* In this generic version, we simply call the ellipse method. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fcircle (R___(Plotter *_plotter) double x, double y, double r)
#else
_g_fcircle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;)
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcircle: invalid operation");
      return -1;
    }

  /* draw only if line type isn't `disconnected' (libplot convention) */
  if (!_plotter->drawstate->points_are_connected)
    {
      /* just move to center of circle, flushing path if any (a libplot
         convention) */
      _plotter->fmove (R___(_plotter) x, y);
      return 0;
    }
  else
    /* invoke `ellipse' method with equal semi-axes; this will first flush
       any stored path, and end by moving to center of ellipse (libplot
       convention) */
    return _plotter->fellipse (R___(_plotter) x, y, r, r, 0.0);
}
