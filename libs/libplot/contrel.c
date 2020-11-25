/* This file contains the contrel() routine, which is a GNU extension to
   libplot.  contrel() is simply a version of cont() that uses relative
   coordinates.  It continues a line. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
contrel (x, y)
     int x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: contrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d\n", 
	     CONTREL, x, y);
  else
    {
      putc (CONTREL, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
    }
  
  return 0;
}

int
fcontrel (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcontrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n", 
	     FCONTREL, x, y);
  else
    {
      putc (FCONTREL, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }
  
  return 0;
}
