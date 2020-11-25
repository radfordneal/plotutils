#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_m_end_page (S___(Plotter *_plotter))
#else
_m_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  _m_emit_op_code (R___(_plotter) O_CLOSEPL);
  _m_emit_terminator (S___(_plotter));

  /* clean up device-specific Plotter members that are heap-allocated */
  if (_plotter->meta_font_name != (const char *)NULL)
    free ((char *)_plotter->meta_font_name);

  if (_plotter->meta_dash_array != (const double *)NULL)
    free ((double *)_plotter->meta_dash_array);

  return true;
}

