/* This file contains the linemod routine, which is a standard part of
   libplot.  It sets a drawing attribute: the line mode used in subsequent
   drawing operations. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
linemod (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linemod() called when graphics device not open\n");
      return -1;
    }

  fprintf (_outstream, "%c%s\n", 
	   LINEMOD, s);

  return 0;
}
