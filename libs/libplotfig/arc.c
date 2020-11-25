/* This file is the arc routine, which is a standard part of libplot.  It
   draws an object: a counterclockwise circular arc with the center at
   xc,yc, the beginning at x0,y0 and the ending at x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

int
farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
{
  double radius;
  Point p0, p1, pc, pb;
  Vector v;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: farc() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  p0.x = x0;
  p0.y = y0;
  p1.x = x1;
  p1.y = y1;
  pc.x = xc;
  pc.y = yc;
  
  /* If affine map from user frame to device frame includes a non-uniform
     scaling, draw inscribed polyline rather than using xfig's native
     circular arc-drawing facility.  xfig has no native support for
     elliptic arcs. */
  if (!_drawstate->transform.uniform)
    {
      _draw_circular_arc (p0, p1, pc);

      endpoly();		/* flush out polyline */
      
      (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
      (_drawstate->pos).y = yc;  

      return 0;
    }

  radius = DIST(pc, p0);

  v.x = p1.x - pc.x;
  v.y = p1.y - pc.y;
  _vscale(&v, radius);
  p1.x = pc.x + v.x;		/* correct the location of p1 */
  p1.y = pc.y + v.y;

  v.x = p1.x - p0.x;		/* vector from p0 to p1 */
  v.y = p1.y - p0.y;
      
  _vscale(&v, radius);
  pb.x = pc.x + v.y;		/* bisection point of arc */
  pb.y = pc.y - v.x;
      
  /* evaluate fig colors lazily, i.e. only when needed */
  _evaluate_fig_fgcolor();
  _evaluate_fig_fillcolor();
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_drawstate->fig_last_priority > ARC_PRIORITY)
    if (_drawstate->fig_drawing_depth > 0)
      (_drawstate->fig_drawing_depth)--;
  _drawstate->fig_last_priority = ARC_PRIORITY;

  sprintf(_outbuf.current,
	  "#ARC\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %.3f %.3f %d %d %d %d %d %d\n",
	  5,			/* arc object */
	  1,			/* open-ended arc subtype */
	  _drawstate->fig_line_style, /* style */
	  _device_line_width(), /* thickness, in Fig display units */
	  _drawstate->fig_fgcolor,	/* pen color */
	  _drawstate->fig_fillcolor, /* fill color */
	  _drawstate->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _drawstate->fig_fill_level, /* area fill */
	  _drawstate->fig_dash_length, /* style val, in Fig display units
					   (float) */
	  _drawstate->fig_cap_style, /* cap style */
	  1,			/* counterclockwise */
	  0,			/* no forward arrow */
	  0,			/* no backward arrow */
	  XD(xc, yc),		/* center_x (float) */
	  YD(xc, yc),		/* center_y (float) */
	  IROUND(XD(p0.x, p0.y)), /* 1st point user entered (p0) */
	  IROUND(YD(p0.x, p0.y)), 
	  IROUND(XD(pb.x, pb.y)), /* 2nd point user entered (bisection point) */
	  IROUND(YD(pb.x, pb.y)),
	  IROUND(XD(p1.x, p1.y)), /* last point user entered (p1, corrected) */
	  IROUND(YD(p1.x, p1.y)));
  _update_buffer (&_outbuf);

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
