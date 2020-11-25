/* This file contains the outfile routine, which is a GNU extension to
   libplot.  It selects an output stream for all subsequent plot commands.
   The outfile routine may only be invoked outside an openpl()...closepl()
   pair.

   In libplotX this function does nothing, since libplotX translates plot
   requests to X calls, rather than writing plot commands to an output
   stream.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

FILE *
outfile(newstream)
     FILE *newstream;
{
  if (_grdevice_open)
    {
      fprintf (stderr, "libplot: outfile() called when graphics device open\n");
      return NULL;
    }

  return 0;
}
