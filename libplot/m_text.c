#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_m_paint_text_string_with_escapes (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_m_paint_text_string_with_escapes (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;
     int v_just;
#endif
{
  _m_set_attributes (R___(_plotter) 
		     PL_ATTR_POSITION
		     | PL_ATTR_TRANSFORMATION_MATRIX 
		     | PL_ATTR_PEN_COLOR | PL_ATTR_PEN_TYPE
		     | PL_ATTR_FONT_NAME | PL_ATTR_FONT_SIZE
		     | PL_ATTR_TEXT_ANGLE);

  _m_emit_op_code (R___(_plotter) O_ALABEL);
  _m_emit_op_code (R___(_plotter) h_just);
  _m_emit_op_code (R___(_plotter) v_just);
  _m_emit_string (R___(_plotter) (const char *)s);
  _m_emit_terminator (S___(_plotter));

  return;
}
