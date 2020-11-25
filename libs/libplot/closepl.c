/* This file contains the closepl routine, which is a standard part of
   libplot.  It closes the graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
closepl ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: closepl() called when graphics device not open\n");
      return -1;
    }
  else
    _grdevice_open = FALSE;

  fflush(_outstream);
  return 0;
}
