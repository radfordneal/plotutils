/* This file contains the label routine, which is a standard part of
   libplot.  It draws a label, i.e. a text string, at the current location
   of the graphics device cursor. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
label (s)
    char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: label() called when graphics device not open\n");
      return -1;
    }

  /* label should have baseline passing through current location, and
     should be left-justified */
  alabel ('l', 'x', s);

  return 0;
}
