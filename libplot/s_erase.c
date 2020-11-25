#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_s_erase_page (S___(Plotter *_plotter))
#else
_s_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* reset page-specific SVGPlotter variables, as if the page had just been
     opened */
  _plotter->s_matrix[0] = 1.0;/* dummy matrix values */
  _plotter->s_matrix[1] = 0.0;
  _plotter->s_matrix[2] = 0.0;
  _plotter->s_matrix[3] = 1.0;
  _plotter->s_matrix[4] = 0.0;
  _plotter->s_matrix[5] = 0.0;
  _plotter->s_matrix_is_unknown = true;
  _plotter->s_matrix_is_bogus = false;

  /* update our knowledge of what SVG's background color should be (we'll
     use it when we write the SVG page header) */
  _plotter->s_bgcolor = _plotter->drawstate->bgcolor;
  _plotter->s_bgcolor_suppressed = _plotter->drawstate->bgcolor_suppressed;

  return true;
}

