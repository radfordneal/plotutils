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

/* For libplotX, we first check to see if the angle of inclination of the
   quarter-ellipse (or three-quarter ellipse) is zero, and the map from
   user coordinates to device coordinates preserves axes.  If both are
   true, we use native X rendering to draw the ellipse.  Otherwise we
   simply call _draw_elliptic_arc() to approximate the elliptic arc by
   drawing an inscribed polyline. */

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

  if ((x0 == xc && y0 == yc) || ((x1 == xc) && (y1 == yc)))
    {
      fprintf (stderr, "libplot: fellarc() called with coincident points as arguments\n");
      return -1;
    }

  if (_drawstate->transform.axes_preserved 
      && ((y0 == yc && x1 == xc)
	  || (x0 == xc && y1 == yc)))
    {				/* CASE 1: use native X arc rendering  */
      int xorigin, yorigin;
      unsigned int squaresize_x, squaresize_y;
      int startangle, endangle, range;
      double rx, ry;

      if (y0 == yc && x1 == xc)
	{	/* initial pt. on x-axis, final pt. on y-axis */
	  startangle = (x0 > xc) ? 0 : 180;
	  endangle = (y1 > yc) ? 90 : 270;
	  rx = (x0 > xc) ? x0 - xc : xc - x0;
	  ry = (y1 > yc) ? y1 - yc : yc - y1;
	}
      else      /* initial pt. on y-axis, final pt. on x-axis */
	{	
	  startangle = (y0 > yc) ? 90 : 270;
	  endangle = (x1 > xc) ? 0 : 180;
	  rx = (x1 > xc) ? x1 - xc : xc - x1;
	  ry = (y0 > yc) ? y0 - yc : yc - y0;
	}	  

      if (endangle < startangle)
	endangle += 360;
      range = endangle - startangle;

      xorigin = IROUND(XD(xc - rx, yc + ry));
      yorigin = IROUND(YD(xc - rx, yc + ry));
      squaresize_x = (unsigned int)IROUND(XDV(2 * rx, 0.0));
      /* note flipped-y convention */
      squaresize_y = (unsigned int)IROUND(YDV(0.0, -2 * ry));
      
      if (_drawstate->fill_level)	/* not transparent */
	{
	  _evaluate_x_fillcolor();
	  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fillcolor);
	  XFillArc(_xdata.dpy, _xdata.window, _drawstate->gc, 
		   xorigin, yorigin, squaresize_x, squaresize_y,
		   64 * startangle, 64 * range);
	  XFillArc(_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		   xorigin, yorigin, squaresize_x, squaresize_y,
		   64 * startangle, 64 * range);
	}
      
      _evaluate_x_fgcolor();
      XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
      XDrawArc(_xdata.dpy, _xdata.window, _drawstate->gc, 
	       xorigin, yorigin, squaresize_x, squaresize_y,
	       64 * startangle, 64 * range);
      XDrawArc(_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
	       xorigin, yorigin, squaresize_x, squaresize_y,
	       64 * startangle, 64 * range);
    }

  else				/* CASE 2: inscribe a polyline in the arc */
    {
      _draw_elliptic_arc (p0, p1, pc); 
      endpoly();		/* flush out polyline */
    }

  (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = yc;

  _handle_x_events();

  return 0;
}

int
ellarc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  return fellarc ((double)xc, (double)yc, (double)x0, (double)y0, (double)x1, (double)y1);
}

