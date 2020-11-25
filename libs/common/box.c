/* This file contains the box routine, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fbox() called when graphics device not open\n");
      return -1;
    }

  fmove (x0, y0);
  fcont (x0, y1);
  fcont (x1, y1);
  fcont (x1, y0);
  fcont (x0, y0);
  fmove (x1, y1);

  return 0;
}

int
box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  return fbox ((double)x0, (double)y0, (double)x1, (double)y1);
}
