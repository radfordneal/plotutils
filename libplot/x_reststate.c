/* This file contains the restorestate method, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by invoking the savestate() method. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_restorestate(void)
#else
_x_restorestate()
#endif
{
  State *oldstate = _plotter->drawstate->previous;

  if (!_plotter->open)
    {
      _plotter->error ("restorestate: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->previous == NULL)
    {
      _plotter->error ("restorestate: invalid operation");
      return -1;
    }

  /* flush polyline if any */
  if (_plotter->drawstate->PointsInLine > 0
      || _plotter->drawstate->arc_stashed)
    _plotter->endpath(); /* flush polyline if any */

  /* elements of current state that are strings are first freed */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  /* N.B. we do _not_ attempt to free _plotter->drawstate->x_font_struct */

  /* free graphics context, if we have one -- and to have one (see
     x_savestate.c), must have at least one drawable */
  if (_plotter->drawable1 || _plotter->drawable2)
    XFreeGC (_plotter->dpy, _plotter->drawstate->gc);

  /* pop current state off the stack */
  free (_plotter->drawstate);
  _plotter->drawstate = oldstate;

  _handle_x_events();

  return 0;
}
