/* This file contains the pointrel routine, which is a GNU extension to
   libplot.  pointrel() is simply a version of point() that uses relative
   coordinates.  It plots an object: a point with specified coordinates. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
pointrel (x, y)
     int x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: pointrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d\n", 
	     POINTREL, x, y);
  else
    {
      putc (POINTREL, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
    }
      
  return 0;
}

int
fpointrel (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fpointrel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n", 
	     FPOINTREL, x, y);
  else
    {
      putc (FPOINTREL, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }
      
  return 0;
}
