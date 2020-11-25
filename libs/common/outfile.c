/* This file contains the outfile routine, which is a GNU extension to
   libplot.  It selects an output stream for all subsequent plot commands.
   The outfile routine may only be invoked outside an openpl()...closepl()
   pair. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* the output stream for all plot commands */

FILE *_outstream = stdout;

/* OUTFILE takes a single argument, a stream that has been opened for writing.
   It may be called only outside an openpl()....closepl() pair.
   It switches all future output to the new, specified stream.  The old
   output stream, which is not closed, is returned. */

FILE *
outfile(newstream)
     FILE *newstream;
{
  FILE *oldstream;
  
  if (_grdevice_open)
    {
      fprintf (stderr, "libplot: outfile() called when graphics device open\n");
      return NULL;
    }

  oldstream = _outstream;
  _outstream = newstream;
  
  return oldstream;
}
