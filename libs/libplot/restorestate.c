/* This file contains the restorestate routine, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by calling the savestate routine. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
restorestate()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: restorestate() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c\n", 
	     RESTORESTATE);
  else
    putc (RESTORESTATE, _outstream);

  return 0;
}
