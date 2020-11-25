/* This file is the circle routine, which is a standard part of libplot.
   It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fcircle (x, y, radius)
     double x, y, radius;
{
  return fellipse (x, y, radius, radius, 0.0);
}

int
circle (x, y, radius)
     int x, y, radius;
{
  return fcircle ((double)x, (double)y, (double)radius);
}

