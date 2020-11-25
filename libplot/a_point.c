/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

/* In the AIPlotter class, a `point' is displayed as a small filled circle
   (one of libplot's standard marker symbols). */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_a_paint_point (S___(Plotter *_plotter))
#else
_a_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double norm;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* compute size of a `point' in user coordinates */
      norm = _matrix_norm (_plotter->drawstate->transform.m);
      if (norm != 0.0)
	{
	  double user_size;
	  
	  user_size = POINT_AI_SIZE / _matrix_norm (_plotter->drawstate->transform.m);
	  _plotter->paint_marker (R___(_plotter) 
			         (int)M_FILLED_CIRCLE, user_size);
	}
    }
}
