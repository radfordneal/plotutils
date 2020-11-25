/* This file contains the pointrel routine, which is a GNU extension to
   libplot.  pointrel() is simply a version of point() that uses relative
   coordinates.  It plots an object: a point with specified coordinates. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fpointrel (dx, dy)
     double dx, dy;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fpointrel() called when graphics device not open\n");
      return -1;
    }

  fpoint ((_drawstate->pos).x + dx, (_drawstate->pos).y + dy);
  
  return 0;
}

int
pointrel (dx, dy)
     int dx, dy;
{
  return fpointrel ((double)dx, (double)dy);
}
