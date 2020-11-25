/* This file contains the marker routine, which is a GNU extension to
   libplot.  It plots an object: a plotting symbol, or marker, of specified
   type and size, at a specified location.  This symbol can be one of a
   list of standard symbols, or a single printable ASCII character in the
   current font and fontsize. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
marker (x, y, type, size)
     int x, y;
     int type;
     int size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: marker() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d\n",
	     MARKER, x, y, type, size);
  else
    {
      putc (MARKER, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (type, _outstream);
      _putshort (size, _outstream);
    }
  
  return 0;
}

int
fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmarker() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %d %g\n",
	     FMARKER, x, y, type, size);
  else
    {
      putc (FMARKER, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
      _putshort (type, _outstream);
      _putfloat (size, _outstream);
    }
  
  return 0;
}
