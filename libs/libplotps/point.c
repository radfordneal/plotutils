/* This file contains the point routine, which is a standard part of libplot.
   It plots an object: a point with coordinates x,y. */

/* In libplotps we display a point as a small circle (a marker symbol). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define POINT_PS_SIZE 0.01  /* size of a `point' in PS device coordinates */

int
fpoint (x, y)
     double x, y;
{
  double norm;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fpoint() called when graphics device not open\n");
      return -1;
    }

  /* compute size of a `point' in user coordinates */
  norm = _matrix_norm (_drawstate->transform.m);
  if (norm != 0.0)
    {
      double user_size;

      user_size = POINT_PS_SIZE / _matrix_norm (_drawstate->transform.m);
      fmarker (x, y, M_FILLED_CIRCLE, user_size);
    }

  return 0;
}

int
point (x, y)
     int x, y;
{
  return fpoint ((double)x, (double)y);
}
