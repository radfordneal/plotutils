/* This file is the circle routine, which is a standard part of libplot.
   It draws an object: a circle with center x,y and radius r. */

/* For libplottek, we call _draw_ellipse() to draw two semicircles, or
   rather approximate them by drawing inscribed polylines using the sagitta
   algorithm.  This is possibly less fast than the midpoint circle
   algorithm (see Foley and van Dam) would be.  However, it's important to
   maintain consistency between the arcs drawn by circle() and the arcs
   drawn by arc().  In libplottek, the latter are also drawn by the sagitta
   algorithm. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fcircle (x, y, r)
     double x, y, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcircle() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _draw_ellipse (x, y, r, r, 0.0);

  endpoly();			/* flush out polyline */
  
  (_drawstate->pos).x = x;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = y;

  return 0;
}

int
circle (x, y, r)
     int x, y, r;
{
  return fcircle ((double)x, (double)y, (double)r);
}
