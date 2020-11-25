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
   returned to at that time. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_savestate (void)
#else
_m_savestate ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("savestate: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c\n", 
		 SAVESTATE);
      else
	putc (SAVESTATE, _plotter->outstream);
    }
  
  /* invoke generic method */
  return _g_savestate ();
}
