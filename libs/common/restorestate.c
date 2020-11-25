/* This file contains the restorestate routine, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by calling the savestate() routine.

   This version of restorestate() assumes that the device-specific part of
   the state contains no strings or other dynamically allocated data.
   Versions of libplot in which this is not true must not use this version,
   since they need to call free() to deallocate space for the strings. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
restorestate()
{
  State *oldstate = _drawstate->previous;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: restorestate() called when graphics device not open\n");
      return -1;
    }

  if (_drawstate->previous == NULL)
    {
      fprintf (stderr, "libplot: restorestate() called on an empty state stack\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  /* elements of current state that are strings are first freed */
  free (_drawstate->line_mode);
  free (_drawstate->join_mode);
  free (_drawstate->cap_mode);
  free (_drawstate->font_name);
  
  /* pop current state off the stack */
  free (_drawstate);
  _drawstate = oldstate;

  return 0;
}
