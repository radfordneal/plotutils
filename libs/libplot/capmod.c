/* This file contains the capmod routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the cap mode used when drawing
   subsequent lines and polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
capmod (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: capmod() called when graphics device not open\n");
      return -1;
    }

  fprintf (_outstream, "%c%s\n", 
	   CAPMOD, s);
  
  return 0;
}
