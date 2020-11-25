/* This file is the arc routine, which is a standard part of libplot.  It
   draws an object: a counterclockwise circular arc with the center at
   pc=(xc,yc), the beginning at p0=(x0,y0) and the ending at p1=(x1,y1).

   In this version, we call _fakearc() to draw an inscribed polyline as an
   approximation to the arc, using move() and cont(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
{
  /* counterclockwise arc,
     center: xc,yc
     from: x0,y0
     to: x1,y1 */
  Point pc, p0, p1;

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: farc() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _draw_circular_arc (p0, p1, pc);

  endpoly();			/* flush out polyline */

  (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = yc;

  return 0;
}

int
arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  return farc ((double)xc, (double)yc, (double)x0, (double)y0, (double)x1, (double)y1);
}
