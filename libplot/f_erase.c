/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   A FigPlotter simply resets the output buffer, discarding all objects
   written to it, and forgets all user-defined colors.  */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_f_erase (void)
#else
_f_erase ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  _reset_outbuf (_plotter->page);

  /* reset our knowledge of xfig's internal state */
  _plotter->fig_drawing_depth = FIG_INITIAL_DEPTH;
  _plotter->fig_num_usercolors = 0;

  /* on to next frame (for a Fig Plotter, which doesn't plot in real time,
     only the last frame in the page is meaningful) */
  _plotter->frame_number++;

  return 0;
}
