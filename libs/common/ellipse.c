/* This file contains the ellipse routine, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

/* In this version, we simply call _draw_ellipse() to approximate the
   ellipse by drawing an inscribed polyline. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipse() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _draw_ellipse (xc, yc, rx, ry, angle);

  (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = yc;

  return 0;
}

int
ellipse (x, y, xradius, yradius, angle)
     int x, y, xradius, yradius, angle;
{
  return fellipse ((double)x, (double)y, 
		   (double)xradius, (double)yradius, (double)angle);
}

