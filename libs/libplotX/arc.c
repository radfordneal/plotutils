/* This file contains the arc routine, which is a standard part of libplot.
   It draws an object: a counterclockwise circular arc with the center at
   xc,yc, the beginning at x0,y0 and the ending at x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
void _draw_elliptic_X_arc P__((Point p0, Point p1, Point pc));
#undef P__

/* If the transformation from user to device coordinates preserves axes,
   the circular arc will be mapped onto an elliptic arc, the axes of the
   ellipse being aligned with the coordinate axes.  In this case we call
   _draw_elliptic_X_arc() to use the native X ellipse drawing facility.
   Otherwise we call the generic routine _draw_circular_arc(), which
   inscribes a polyline using move() and cont(). */
void
_draw_elliptic_X_arc (p0, p1, pc)
     Point p0, p1, pc;
{
  Vector v;
  double theta0, theta1;
  double radius;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;
  int startangle, anglerange;

  /* radius is distance to p0; we adjust location of p1 to match */
  radius = DIST(pc, p0);

  v.x = p1.x - pc.x;
  v.y = p1.y - pc.y;
  _vscale(&v, radius);
  p1.x = pc.x + v.x;		/* correct the location of p1 */
  p1.y = pc.y + v.y;

  theta0 = _xatan2 (p0.y - pc.y, p0.x - pc.x);
  theta1 = _xatan2 (p1.y - pc.y, p1.x - pc.x);

  if (theta0 > theta1) 
    theta1 += 2 * M_PI;		/* so that difference > 0 */
  if (theta0 < 0.0)
    {
      theta0 += 2 * M_PI;	/* so that startangle > 0 */
      theta1 += 2 * M_PI;
    }

  /* dimensions, in device frame (raster coordinates) */
  xorigin = IROUND(XD(pc.x - radius, pc.y + radius));
  yorigin = IROUND(YD(pc.x - radius, pc.y + radius));
  squaresize_x = (unsigned int)IROUND(XDV(2 * radius, 0.0));
  /* flipped-y convention used here */
  squaresize_y = (unsigned int)IROUND(YDV(0.0, -2 * radius));
  startangle = IROUND(64 * theta0 * 180.0 / M_PI); /* in 64'ths of a degree */
  anglerange = IROUND(64 * (theta1 - theta0) * 180.0 / M_PI); /* likewise */

  if (_drawstate->fill_level)	/* not transparent */
    {
      _evaluate_x_fillcolor();
      XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fillcolor);
      XFillArc (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange);
      XFillArc (_xdata.dpy, _xdata.window, _drawstate->gc, 
		xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange);
    }

  _evaluate_x_fgcolor();
  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
  XDrawArc (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
	    xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange);
  XDrawArc (_xdata.dpy, _xdata.window, _drawstate->gc, 
	    xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange);
  
  return;
}

int
farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
{
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

  /* If transformation from user to device coordinates does not preserve
     axes, draw inscribed polyline rather than using native X rendering. */
  if (!_drawstate->transform.axes_preserved)
    {
      _draw_circular_arc(p0, p1, pc);
      endpoly();		/* flush out polyline */
    }
  else
    _draw_elliptic_X_arc (p0, p1, pc);
  
  (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = yc;

  _handle_x_events();
  return 0;
}

int
arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
{
  return farc ((double)xc, (double)yc, (double)x0, (double)y0, (double)x1, (double)y1);
}
