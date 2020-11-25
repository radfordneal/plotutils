#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_m_erase_page (S___(Plotter *_plotter))
#else
_m_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  _m_set_attributes (R___(_plotter) 
		     PL_ATTR_BG_COLOR);
  _m_emit_op_code (R___(_plotter) O_ERASE);
  _m_emit_terminator (S___(_plotter));

  return true;
}

