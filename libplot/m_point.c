/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_m_paint_point (S___(Plotter *_plotter))
#else
_m_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  _m_set_attributes (R___(_plotter) 
		     PL_ATTR_TRANSFORMATION_MATRIX 
		     | PL_ATTR_PEN_COLOR | PL_ATTR_PEN_TYPE);
  _m_emit_op_code (R___(_plotter) O_FPOINT);
  _m_emit_float (R___(_plotter) _plotter->drawstate->pos.x);
  _m_emit_float (R___(_plotter) _plotter->drawstate->pos.y);
  _m_emit_terminator (S___(_plotter));

  _plotter->meta_pos = _plotter->drawstate->pos;

  return;
}

