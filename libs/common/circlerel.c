/* This file is the circlerel routine, which is a GNU extension to libplot.
   circlerel() is simply a version of circle() that uses relative
   coordinates.  It draws an object: a circle with specified center and
   radius.

   This file also contains the ellipsrel() routine, which is an extension
   to libplot. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fcirclerel (dx, dy, r)
     double dx, dy, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcirclerel() called when graphics device not open\n");
      return -1;
    }

  return fcircle ((_drawstate->pos).x + dx, (_drawstate->pos).y + dy, r);
}

int
circlerel (dx, dy, r)
     int dx, dy, r;
{
  return fcirclerel ((double)dx, (double)dy, (double)r);
}

int
fellipserel (dx, dy, rx, ry, angle)
     double dx, dy, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipserel() called when graphics device not open\n");
      return -1;
    }

  return fellipse ((_drawstate->pos).x + dx, (_drawstate->pos).y + dy, rx, ry, angle);
}

int
ellipserel (dx, dy, rx, ry, angle)
     int dx, dy, rx, ry, angle;
{
  return fellipserel ((double)dx, (double)dy, (double)rx, (double)ry, (double)angle);
}
