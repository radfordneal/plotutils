/* This file contains the markerrel routine, which is a GNU extension to
   libplot.  markerrel() is simply a version of marker() that uses relative
   coordinates.  It plots an object: a plotting symbol, or marker, of
   specified type and size. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fmarkerrel (dx, dy, type, size)
     double dx, dy;
     int type;
     double size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmarkerrel() called when graphics device not open\n");
      return -1;
    }

  fmarker ((_drawstate->pos).x + dx, (_drawstate->pos).y + dy, type, size);
  
  return 0;
}

int
markerrel (dx, dy, type, size)
     int dx, dy;
     int type;
     int size;
{
  return fmarkerrel ((double)dx, (double)dy, type, (double)size);
}
