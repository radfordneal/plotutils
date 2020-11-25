/* This file contains the point routine, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* In libplottek we display a point as a zero-length line segment. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fpoint (x, y)
     double x, y;
{
  double xx, yy;
  int ixx, iyy;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fpoint() called when graphics device not open\n");
      return -1;
    }

  endpoly();			/* flush polyline if any */
  
  /* update our notion of position */
  (_drawstate->pos).x = x;
  (_drawstate->pos).y = y;

  /* convert point to floating-point device coordinates */
  xx = XD(x,y);
  yy = YD(x,y);

  /* do nothing if point is outside device clipping rectangle */
  if ((xx < DEVICE_X_MIN_CLIP)
      || (xx > DEVICE_X_MAX_CLIP)
      || (yy < DEVICE_Y_MIN_CLIP)
      || (yy > DEVICE_Y_MAX_CLIP))
    return 0;

  /* round to integer device (Tektronix) coordinates */
  ixx = IROUND(xx);
  iyy = IROUND(yy);

  _drawstate->tek_mode = MODE_POINT;
  _tek_mode();			/* Tek now agrees with us on mode */
  
  /* Output the point.  If in fact we were already in POINT mode, this is
     slightly suboptimal because we can't call _putcode_compressed() to
     save (potentially) a few bytes, because we don't know what the
     last-plotted point was.  Unlike when incrementally drawing a polyline,
     when plotting points we don't keep track of "where we last were". */
  _putcode (ixx, iyy, _outstream);

  /* update our notion of Tek's notion of position */
  (_tekstate.pos).x = ixx;
  (_tekstate.pos).y = iyy;
  
  return 0;
}

int
point (x, y)
     int x, y;
{
  return fpoint ((double)x, (double)y);
}
