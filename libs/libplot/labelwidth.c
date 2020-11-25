/* This file contains the labelwidth routine, which is a GNU extension to
   libplot.  It returns the width, in user units, of a label, i.e., a text
   string.  A return value of zero may mean that no information on font
   size is available.

   In raw libplot, we return 0, since we have no information about the
   mapping between user coordinates and device coordinates that will
   ultimately be performed. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
flabelwidth (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: flabelwidth() called when graphics device not open\n");
      return -1;
    }

  return 0.0;
}

int 
labelwidth (s)
     char *s;
{
  double width = flabelwidth (s);
  
  return IROUND(width);
}
