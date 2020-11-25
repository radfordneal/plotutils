/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* So far as the AIPlotter class goes, we display a `point' as a small
   circle (a marker symbol). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_a_fpoint (R___(Plotter *_plotter) double x, double y)
#else
_a_fpoint (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  double norm;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fpoint: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* compute size of a `point' in user coordinates */
  norm = _matrix_norm (_plotter->drawstate->transform.m);
  if (norm != 0.0)
    {
      double user_size;

      user_size = POINT_AI_SIZE / _matrix_norm (_plotter->drawstate->transform.m);
      _plotter->fmarker (R___(_plotter) 
			 x, y, (int)M_FILLED_CIRCLE, user_size);
    }

  return 0;
}
