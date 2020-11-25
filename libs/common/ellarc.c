/* This file contains the ellarc routine, which is a GNU extension to
   libplot.  It draws an object: a counterclockwise arc of an ellipse,
   extending from p0=(x0,y0) to p1=(x1,y1).  The center of the ellipse will
   be at pc=(xc,yc).

   These conditions do not uniquely determine the elliptic arc (or the
   ellipse of which it is an arc).  In the case when (p0-pc) x (p1-pc) is
   positive, we choose the elliptic arc so that it has control points p0,
   p1, and p0 + p1 - pc.  This means that it passes through p0 and p1, is
   tangent at p0 to the line segment joining p0 to p0 + p1 - pc, and is
   tangent at p1 to the line segment joining p1 to p0 + p1 - pc.  So it
   fits snugly into a triangle with these three vertices.  Notice that the
   control point p0 + p1 - pc is simply the reflection of pc through the
   line determined by p0 and p1.
   
   This sort of elliptic arc is called a `quarter-ellipse', since it is an
   affinely transformed quarter-circle.  Specifically, it is an affinely
   transformed version of the first quadrant of a unit circle, with
   the affine transformation mapping (0,0) to pc, (0,1) to p0, (1,0) to p1,
   and (1,1) to the control point p0 + p1 - pc.

   All the above applies when (p0-pc) x (p1-pc) is positive.  When it is
   negative, we draw a three-quarter ellipse, the definition of which is
   left to the reader. */

/* In this generic version, we simply call _draw_elliptic_arc() to
   approximate the elliptic arc by drawing an inscribed polyline. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fellarc (xc, yc, x0, y0, x1, y1)
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
      fprintf (stderr, "libplot: fellarc() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _draw_elliptic_arc (p0, p1, pc);

  endpoly();			/* flush out polyline */

  (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = yc;

  return 0;
}

int
ellarc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  return fellarc ((double)xc, (double)yc, (double)x0, (double)y0, (double)x1, (double)y1);
}

