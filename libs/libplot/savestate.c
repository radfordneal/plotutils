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
   returned to at that time. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
savestate ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: savestate() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c\n", 
	     SAVESTATE);
  else
    putc (SAVESTATE, _outstream);
  
  return 0;
}
