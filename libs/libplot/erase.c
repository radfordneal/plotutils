/* This file contains the erase routine, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
erase ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: erase() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c\n", 
	     ERASE);
  else
    putc (ERASE, _outstream);

  return 0;
}
