/* This file contains the savestate method, which is a GNU extension to
   libplot.  It creates a new drawing state and pushes it onto the stack of
   drawing states.  By definition, a `drawing state' comprises the set of
   drawing attributes, and the state of any polyline being incrementally
   drawn.

   The new state will have the same drawing attributes as the old state.
   If a polyline was being drawn incrementally in the old state, the new
   state will not contain it.  The old state may be returned to by invoking
   the restorestate routine, which pops drawing states off the stack.  If
   the incremental drawing of a polyline was in progress, it may be
   returned to at that time.

   This version of savestate() assumes that the device-specific part of the
   drawing state contains no strings.  Plotter objects for which this is
   not true must not use this version, since they need to call malloc() to
   allocate space for the string in the new state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_savestate(void)
#else
_g_savestate()
#endif
{
  State *oldstate = _plotter->drawstate;
  const State *copyfrom;

  if (!_plotter->open)
    {
      _plotter->error ("savestate: invalid operation");
      return -1;
    }

  /* create a new state */
  _plotter->drawstate = (State *)_plot_xmalloc (sizeof(State));
  
  /* copy from old state unless there wasn't any (i.e. an empty stack) */
  copyfrom = oldstate ? oldstate : _plotter->default_drawstate;
  memcpy (_plotter->drawstate, copyfrom, sizeof(State));

  /* elements of state that are strings are treated specially */
  _plotter->drawstate->line_mode = (char *)_plot_xmalloc (strlen (copyfrom->line_mode) + 1);
  _plotter->drawstate->join_mode = (char *)_plot_xmalloc (strlen (copyfrom->join_mode) + 1);
  _plotter->drawstate->cap_mode = (char *)_plot_xmalloc (strlen (copyfrom->cap_mode) + 1);
  _plotter->drawstate->font_name = (char *)_plot_xmalloc (strlen (copyfrom->font_name) + 1);  
  strcpy (_plotter->drawstate->line_mode, copyfrom->line_mode);
  strcpy (_plotter->drawstate->join_mode, copyfrom->join_mode);
  strcpy (_plotter->drawstate->cap_mode, copyfrom->cap_mode);
  strcpy (_plotter->drawstate->font_name, copyfrom->font_name);  

  /* Our memcpy copied the polyline object (if there is one), as well as the
     drawing attributes.  So we knock it out, to start afresh */

  _plotter->drawstate->datapoints = NULL;
  _plotter->drawstate->PointsInLine = 0;
  _plotter->drawstate->datapoints_len = 0;

  /* install new state at head of the state list */
  _plotter->drawstate->previous = oldstate;

  return 0;
}
