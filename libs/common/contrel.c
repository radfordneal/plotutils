/* This file contains the contrel() routine, which is a GNU extension to
   libplot.  contrel() is simply a version of cont() that uses relative
   coordinates.  It continues a line. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fcontrel (dx, dy)
     double dx, dy;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcontrel() called when graphics device not open\n");
      return -1;
    }

  return fcont ((_drawstate->pos).x + dx, (_drawstate->pos).y + dy);
}

int
contrel (x, y)
     int x, y;
{
  return fcontrel ((double)x, (double)y);
}
