/* This file contains the joinmod routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when drawing
   subsequent polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
joinmod (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: joinmod() called when graphics device not open\n");
      return -1;
    }

  fprintf (_outstream, "%c%s\n", 
	   JOINMOD, s);
  
  return 0;
}
