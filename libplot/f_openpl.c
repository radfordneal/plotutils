#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_f_begin_page (S___(Plotter *_plotter))
#else
_f_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* With each call to openpl(), we reset our knowledge of Fig's
     internal state, i.e. the dynamic Fig-specific data members of the
     FigPlotter.  The values are the same as are used in initializing the
     FigPlotter (see f_defplot.c). */
     
  _plotter->fig_drawing_depth = FIG_INITIAL_DEPTH;
  _plotter->fig_num_usercolors = 0;

  return true;
}
