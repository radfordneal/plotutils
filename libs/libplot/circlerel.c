/* This file contains the circlerel routine, which is a GNU extension to
   libplot.  circlerel() is simply a version of circle() that uses relative
   coordinates.  It draws an object: a circle with specified center and
   radius. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
circlerel (x, y, r)
     int x, y, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: circlerel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d\n", 
	     CIRCLEREL, x, y, r);
  else
    {
      putc (CIRCLEREL, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (r, _outstream);
    }

  return 0;
}

int
fcirclerel (x, y, r)
     double x, y, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcirclerel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g\n", 
	     FCIRCLEREL, x, y, r);
  else
    {
      putc (FCIRCLEREL, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
      _putfloat (r, _outstream);
    }

  return 0;
}

int
ellipserel (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ellipserel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d %d\n", 
	     ELLIPSEREL, x, y, rx, ry, angle);
  else
    {
      putc (ELLIPSEREL, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
      _putshort (rx, _outstream);
      _putshort (ry, _outstream);
      _putshort (angle, _outstream);
    }

  return 0;
}

int
fellipserel (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipserel() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g\n", 
	     FELLIPSEREL, x, y, rx, ry, angle);
  else
    {
      putc (FELLIPSEREL, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
      _putfloat (rx, _outstream);
      _putfloat (ry, _outstream);
      _putfloat (angle, _outstream);
    }

  return 0;
}
