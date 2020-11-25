/* This file contains the boxrel routine, which is a GNU extension to
   libplot.  boxrel() is simply a version of box() that uses relative
   coordinates.  It draws an object: an upright rectangle with specified
   diagonal corners. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fboxrel (dx0, dy0, dx1, dy1)
     double dx0, dy0, dx1, dy1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fboxrel() called when graphics device not open\n");
      return -1;
    }

  fbox ((_drawstate->pos).x + dx0, (_drawstate->pos).y + dy0,
	(_drawstate->pos).x + dx1, (_drawstate->pos).y + dy1);
  
  return 0;
}

int
boxrel (dx0, dy0, dx1, dy1)
     int dx0, dy0, dx1, dy1;
{
  return fboxrel ((double)dx0, (double)dy0, (double)dx1, (double)dy1);
}
