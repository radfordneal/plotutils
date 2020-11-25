/* This file contains the restorestate method, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by invoking the savestate routine. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_restorestate(void)
#else
_m_restorestate()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("restorestate: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c\n", 
		 (int)O_RESTORESTATE);
      else
	putc ((int)O_RESTORESTATE, _plotter->outstream);
    }

  /* invoke generic method */
  return _g_restorestate ();
}
