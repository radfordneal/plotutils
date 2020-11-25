/* This file contains the line routine, which is a standard part of
   libplot.  It draws an object: a line segment extending from the point
   x0,y0 to the point x1,y1.  By repeatedly calling cont(), the user may
   extend this line object to a polyline object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fline() called when graphics device not open\n");
      return -1;
    }

  fmove (x0, y0);
  fcont (x1, y1);
  return 0;
}

int
line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  return fline ((double)x0, (double)y0, (double)x1, (double)y1);
}
