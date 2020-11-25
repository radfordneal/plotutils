/* This file contains the savestate routine, which is a GNU extension to
   libplot.  It creates a new drawing state and pushes it onto the stack of
   drawing states.  By definition, a `drawing state' comprises the set of
   drawing attributes, and the state of any polyline being incrementally
   drawn.

   The new state will have the same drawing attributes as the old state.
   If a polyline was being drawn incrementally in the old state, the new
   state will not contain it.  The old state may be returned to by calling
   the restorestate routine, which pops drawing states off the stack.
   If the incremental drawing of a polyline was in progress, it may be
   returned to at that time.

   This version of savestate() assumes that the device-specific part of the
   state contains no strings.  Versions of libplot in which this is not
   true must not use this version, since they need to call malloc() to
   allocate space for the string in the new state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "defaultstate.h"	/* contains initialization of new state */

/* pointer to state at head of linked list */
State *_drawstate = NULL;	/* initially, list is empty */

int
savestate()
{
  State *oldstate = _drawstate;
  State *copyfrom;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: savestate() called when graphics device not open\n");
      return -1;
    }

  /* create a new state */
  _drawstate = (State *)_plot_xmalloc (sizeof(State));
  
  /* copy from old state unless there wasn't any (i.e. an empty stack) */
  copyfrom = oldstate ? oldstate : &_default_drawstate;
  memcpy (_drawstate, copyfrom, sizeof(State));

  /* elements of state that are strings are treated specially */
  _drawstate->line_mode = (char *)_plot_xmalloc (strlen (copyfrom->line_mode) + 1);
  _drawstate->join_mode = (char *)_plot_xmalloc (strlen (copyfrom->join_mode) + 1);
  _drawstate->cap_mode = (char *)_plot_xmalloc (strlen (copyfrom->cap_mode) + 1);
  _drawstate->font_name = (char *)_plot_xmalloc (strlen (copyfrom->font_name) + 1);  
  strcpy (_drawstate->line_mode, copyfrom->line_mode);
  strcpy (_drawstate->join_mode, copyfrom->join_mode);
  strcpy (_drawstate->cap_mode, copyfrom->cap_mode);
  strcpy (_drawstate->font_name, copyfrom->font_name);  

  /* Our memcpy copied the polyline object (if there is one), as well as the
     drawing attributes.  So we knock it out, to start afresh */

  _drawstate->datapoints = NULL;
  _drawstate->PointsInLine = 0;
  _drawstate->datapoints_len = 0;

  /* install new state at head of the state list */
  _drawstate->previous = oldstate;

  return 0;
}
