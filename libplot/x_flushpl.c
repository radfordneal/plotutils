/* This file contains the flushpl method, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

/* This version is for XDrawablePlotters (and XPlotters). */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_x_flush_output (S___(Plotter *_plotter))
#else
_x_flush_output (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  XSync (_plotter->x_dpy, (Bool)false);

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return true;
}
