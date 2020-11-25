/* This file contains the restorestate method, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by invoking the savestate() method. */

/* This version is for XDrawablePlotters (and XPlotters). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_restorestate(S___(Plotter *_plotter))
#else
_x_restorestate(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  plDrawState *oldstate = _plotter->drawstate->previous;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "restorestate: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->previous == NULL)
    /* this is an attempt to pop the lowest state off the stack */
    {
      _plotter->error (R___(_plotter) "restorestate: invalid operation");
      return -1;
    }

  /* flush polyline if any */
  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* elements of current state that are strings are first freed */
  free ((char *)_plotter->drawstate->fill_rule);
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->font_name);
  
  /* free dash array too, if nonempty */
  if (_plotter->drawstate->dash_array_len > 0)
    free ((double *)_plotter->drawstate->dash_array);

  /* N.B. we do _not_ free _plotter->drawstate->x_font_struct */

  /* Free graphics contexts, if we have them -- and to have them, must have
     at least one drawable (see x_savestate.c). */
  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      /* free the dash list in the X11-specific part of the drawing state */
      if (_plotter->drawstate->x_gc_dash_list_len > 0
	  && _plotter->drawstate->x_gc_dash_list != (char *)NULL)
	free ((char *)_plotter->drawstate->x_gc_dash_list);

      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_bg);
    }

  /* pop current state off the stack */
  free (_plotter->drawstate);
  _plotter->drawstate = oldstate;

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return 0;
}
