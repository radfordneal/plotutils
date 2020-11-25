/* This file contains the markerrel routine, which is a GNU extension to
   libplot.  markerrel() is simply a version of marker() that uses relative
   coordinates.  It plots an object: a plotting symbol, or marker, of
   specified type and size. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
markerrel (x, y, type, size)
     int x, y;
     int type;
     int size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: markrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d\n",
	     MARKERREL, x, y, type, size);
  else
    {
      putc (MARKERREL, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (type, _outstream);
      _putshort (size, _outstream);
    }
  
  return 0;
}

int
fmarkerrel (x, y, type, size)
     double x, y;
     int type;
     double size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmarkerrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %d %g\n",
	     FMARKERREL, x, y, type, size);
  else
    {
      putc (FMARKERREL, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
      _putshort (type, _outstream);
      _putfloat (size, _outstream);
    }
  
  return 0;
}
