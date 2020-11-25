/* The paint_marker method, which is an internal function that is called
   when the marker() method is invoked.  It plots an object: a marker of a
   specified type, at a specified size, at the current location.

   If this returns `false', marker() will construct the marker from other
   libplot primitives, in a generic way. */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_m_paint_marker (R___(Plotter *_plotter) int type, double size)
#else
_m_paint_marker (R___(_plotter) type, size)
     S___(Plotter *_plotter;)
     int type;
     double size;
#endif
{
  _m_set_attributes (R___(_plotter) 
		     PL_ATTR_TRANSFORMATION_MATRIX 
		     | PL_ATTR_PEN_COLOR | PL_ATTR_PEN_TYPE);
  _m_emit_op_code (R___(_plotter) O_FMARKER);
  _m_emit_float (R___(_plotter) _plotter->drawstate->pos.x);
  _m_emit_float (R___(_plotter) _plotter->drawstate->pos.y);
  _m_emit_integer (R___(_plotter) type);
  _m_emit_float (R___(_plotter) size);
  _m_emit_terminator (S___(_plotter));

  _plotter->meta_pos = _plotter->drawstate->pos;

  return true;
}


