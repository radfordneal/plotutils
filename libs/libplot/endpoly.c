/* This file contains the endpoly() routine, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated calls to the cont() routine.  (See the comments in cont.c.)
   The construction may be terminated, and the polyline object finalized,
   by an explict call to endpoly().

   If endpoly() is called when no polyline is under construction, it
   has no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
endpoly ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: endpoly() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c\n", 
	     ENDPOLY);
  else
    putc (ENDPOLY, _outstream);

  return 0;
}
