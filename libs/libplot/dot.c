/* This file contains the dot routine, which is a standard (though
   undocumented and obsolete) part of libplot.  I haven't yet found out
   exactly what `dot' is supposed to do, or ever did, so it is not
   implemented fully.  It appears from old BSD source that it was specific
   to a single device.  See ../common/dot.c for more details. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
dot (x, y, dx, n, pattern)
     int x, y, dx, n;
     char *pattern;
{
  int i;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: dot() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    {
      fprintf (_outstream, "%c %d %d %d %d ", 
	       DOT, x, y, dx, n);
      fwrite (pattern, 1, (unsigned int)n, _outstream);
      fprintf (_outstream, "\n");
    }    
  else
    {
      putc (DOT, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (dx, _outstream);
      _putshort (n, _outstream);
      for (i = 0; i < n; i++)
	_putshort (*pattern++, _outstream);
    }

  return 0;
}
