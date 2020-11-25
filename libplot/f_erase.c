#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_f_erase_page (S___(Plotter *_plotter))
#else
_f_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* reset our knowledge of xfig's internal state */
  _plotter->fig_drawing_depth = FIG_INITIAL_DEPTH;
  _plotter->fig_num_usercolors = 0; /* forget user-defined colors */

  return true;
}
