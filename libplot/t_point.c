/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* TekPlotter objects display a point as a zero-length line segment. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_t_fpoint (double x, double y)
#else
_t_fpoint (x, y)
     double x, y;
#endif
{
  double xx, yy;
  int ixx, iyy;

  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  _plotter->endpath(); /* flush polyline if any */
  
  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  /* convert point to floating-point device coordinates */
  xx = XD(x,y);
  yy = YD(x,y);

  /* do nothing if point is outside device clipping rectangle */
  if ((xx < TEK_DEVICE_X_MIN_CLIP)
      || (xx > TEK_DEVICE_X_MAX_CLIP)
      || (yy < TEK_DEVICE_Y_MIN_CLIP)
      || (yy > TEK_DEVICE_Y_MAX_CLIP))
    return 0;

  /* round to integer device (Tektronix) coordinates */
  ixx = IROUND(xx);
  iyy = IROUND(yy);

  /* emit an escape sequence if necessary, to switch to POINT mode */
  _tek_mode (MODE_POINT);
  
  /* Output the point.  If in fact we were already in POINT mode, this is
     slightly suboptimal because we can't call _tek_vector_compressed() to
     save (potentially) a few bytes, because we don't know what the
     last-plotted point was.  Unlike when incrementally drawing a polyline,
     when plotting points we don't keep track of "where we last were". */
  _tek_vector (ixx, iyy);

  /* update our notion of Tek's notion of position */
  _plotter->tek_pos.x = ixx;
  _plotter->tek_pos.y = iyy;
  
  return 0;
}
