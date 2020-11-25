/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for Plotters that do not plot in real time, but emit a
   page of graphics when closepl() is called. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_f_openpl (S___(Plotter *_plotter))
#else
_f_openpl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->open)
    {
      _plotter->error (R___(_plotter) "openpl: invalid operation");
      return -1;
    }

  /* Prepare buffer in which we'll cache graphics code for this page.
     Although we won't be emitting graphics in real time, we don't maintain
     a linked list of buffers, one per page; we don't need to. */
  _plotter->page = _new_outbuf ();
  
  /* With each call to openpl(), we reset our knowledge of Fig's
     internal state, i.e. the dynamic Fig-specific data members of the
     FigPlotter.  The values are the same as are used in initializing the
     FigPlotter (see f_defplot.c). */
     
  _plotter->fig_drawing_depth = FIG_INITIAL_DEPTH;
  _plotter->fig_num_usercolors = 0;
  
  /* invoke generic method, to e.g. create drawing state */
  _g_openpl (S___(_plotter));

  return 0;
}
