/* This file contains the linerel() routine, which is a GNU extension to
   libplot.  linerel() is simply a version of line() that uses relative
   coordinates.  It draws an object: a line segment extending from one
   specified point to another.  By repeatedly calling cont(), the user may
   extend this line object to a polyline object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
flinerel (dx0, dy0, dx1, dy1)
     double dx0, dy0, dx1, dy1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: flinerel() called when graphics device not open\n");
      return -1;
    }

  fline ((_drawstate->pos).x + dx0, (_drawstate->pos).y + dy0,
	(_drawstate->pos).x + dx1, (_drawstate->pos).y + dy1);
  
  return 0;
}

int
linerel (dx0, dy0, dx1, dy1)
     int dx0, dy0, dx1, dy1;
{
  return flinerel ((double)dx0, (double)dy0, (double)dx1, (double)dy1);
}
