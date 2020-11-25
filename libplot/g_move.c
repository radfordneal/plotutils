/* This file contains the move method, which is a standard part of libplot.
   It sets a drawing attribute: the location of the graphics cursor, which
   determines the position of the next object drawn on the graphics
   device. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_fmove (R___(Plotter *_plotter) double x, double y)
#else
_API_fmove (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     double x, y;
#endif
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmove: invalid operation");
      return -1;
    }

  /* flush path under construction, if any */
  if (_plotter->drawstate->path)
    _API_endpath (S___(_plotter));

  _plotter->drawstate->pos.x = x; /* update our notion of position */
  _plotter->drawstate->pos.y = y;

  return 0;
}
