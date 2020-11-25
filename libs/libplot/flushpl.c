/* This file contains the flushpl routine, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
flushpl ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: flushpl() called when graphics device not open\n");
      return -1;
    }

  fflush (_outstream);
  return 0;
}
