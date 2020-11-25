/* This file contains the restorestate method, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by invoking the savestate() routine.

   This version of restorestate() assumes that the device-specific part of
   the state contains no strings or other dynamically allocated data.
   Versions of libplot in which this is not true must not use this version,
   since they need to call free() to deallocate space for the strings. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_restorestate(void)
#else
_g_restorestate()
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

  /* Flush polyline if any.  We first check whether a polyline is under
     construction, because we don't want to emit a useless op code to a
     graphics metafile, if this is invoked on a MetaPlotter object. */
  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* elements of current state that are strings are first freed */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  /* pop current state off the stack */
  free (_plotter->drawstate);
  _plotter->drawstate = oldstate;

  return 0;
}
