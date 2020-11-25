/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

int
#ifdef _HAVE_PROTOS
_f_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_f_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point p0, p1, pc, pm, pb;
  Vector v, v0, v1;
  double cross, radius;
  int orientation;
  
  if (!_plotter->open)
    {
      _plotter->error ("farc: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x1,y1).  Only the endpoints will appear on the
     display.  If linemode is "disconnected", no arc-stashing (see below)
     ever occurs. */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (x0, y0, x1, y1);

  /* Non-immediate case: an arc was previously stashed rather than drawn,
     so first flush it out by drawing a polygonal approximation to it.  We
     do this by setting the `immediate' flag and making a recursive call. */
  if (!_plotter->drawstate->arc_immediate
      && _plotter->drawstate->arc_stashed)
    {
      double axc = _plotter->drawstate->axc;
      double ayc = _plotter->drawstate->ayc;
      double ax0 = _plotter->drawstate->ax0;
      double ay0 = _plotter->drawstate->ay0;
      double ax1 = _plotter->drawstate->ax1;
      double ay1 = _plotter->drawstate->ay1;

      _plotter->drawstate->arc_immediate = true;
      _plotter->drawstate->arc_polygonal = true;
      _plotter->farc (axc, ayc, ax0, ay0, ax1, ay1);
      _plotter->drawstate->arc_immediate = false;
      _plotter->drawstate->arc_stashed = false;
    }

  /* Non-immediate case: if new arc not contiguous, move to its starting
     point (thereby finalizing the path under construction, if any, since
     the move() method invokes the endpath() method). */
  if ((!_plotter->drawstate->arc_immediate)
      && (x0 != _plotter->drawstate->pos.x 
	  || y0 != _plotter->drawstate->pos.y))
    _plotter->fmove (x0, y0);
  
  if (!_plotter->drawstate->arc_immediate
      && _plotter->drawstate->PointsInLine == 0)
    /* Non-immediate case, with no polyline under construction:
       don't actually draw the arc, just stash it for later drawing */
    {
      _plotter->drawstate->axc = xc;
      _plotter->drawstate->ayc = yc;      
      _plotter->drawstate->ax0 = x0;
      _plotter->drawstate->ay0 = y0;      
      _plotter->drawstate->ax1 = x1;
      _plotter->drawstate->ay1 = y1;      
      _plotter->drawstate->arc_stashed = true;
      return 0;
    }

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  /* Immediate case, or non-immediate case with a polyline under
     construction.  We actually draw the arc.  In the immediate case we
     check the advisory `arc_polygonal' flag to determine whether to draw a
     polygonal approximation.  But we only check it if the map from the
     user frame to the device frame is isotropic. */

  /* if affine map from user frame to device frame is anisotropic, draw a
     polygonal approximation by invoking fcont() repeatedly */
  if (!_plotter->drawstate->transform.uniform
      || (_plotter->drawstate->arc_immediate
	  && _plotter->drawstate->arc_polygonal)
      || _plotter->drawstate->PointsInLine > 0)
    {
      _plotter->drawstate->arc_stashed = false;	/* avoid infinite recursion */
      _draw_circular_arc (p0, p1, pc);
      return 0;
    }

  /* otherwise use xfig's native arc-drawing facility, as follows */

  if (x0 == x1 && y0 == y1)
    /* zero-length arc */
    {
      _plotter->fmove (x0, y0);
      _plotter->fcont (x0, y0);
      _plotter->endpath (); /* flush out zero-length line */

      _plotter->drawstate->pos = p1; /* move to p1 (a libplot convention) */

      return 0;
    }

  /* genuine arc, adjust pc as needed */
  pc = _truecenter (p0, p1, pc);

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

  /* cross product, zero means points are collinear */
  cross = v0.x * v1.y - v1.x * v0.y;

  /* Compute orientation.  Note libplot convention: if p0, p1, pc are
     collinear then arc goes counterclockwise from p0 to p1. */
  orientation = (cross >= 0.0 ? 1 : -1);

  radius = DIST(pc, p0);	/* radius is distance to p0 or p1 */

  pm.x = 0.5 * (p0.x + p1.x);	/* midpoint of chord from p0 to p1 */
  pm.y = 0.5 * (p0.y + p1.y);  

  v.x = p1.x - p0.x;		/* chord vector from p0 to p1 */
  v.y = p1.y - p0.y;
      
  _vscale(&v, radius);
  pb.x = pc.x + orientation * v.y; /* bisection point of arc */
  pb.y = pc.y - orientation * v.x;
      
  /* evaluate fig colors lazily, i.e. only when needed */
  _plotter->set_pen_color();
  _plotter->set_fill_color();
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_plotter->fig_last_priority >= ARC_PRIORITY)
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;
  _plotter->fig_last_priority = ARC_PRIORITY;

  /* compute orientation in device frame */
  orientation *= (_plotter->drawstate->transform.nonreflection ? 1 : -1);

  if (orientation == -1)
    /* interchange p0, p1 (since xfig insists that p0, pb, p1 must appear
       in counterclockwise order around the arc) */
    {
      Point ptmp;
      
      ptmp = p0;
      p0 = p1;
      p1 = ptmp;
    }

  sprintf(_plotter->outbuf.current,
	  "#ARC\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %.3f %.3f %d %d %d %d %d %d\n",
	  5,			/* arc object */
	  1,			/* open-ended arc subtype */
	  _fig_line_style[_plotter->drawstate->line_type], /* style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  			/* style val, in Fig display units (float) */
	  _fig_dash_length[_plotter->drawstate->line_type], 
	  _fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
	  1,			/* counterclockwise */
	  0,			/* no forward arrow */
	  0,			/* no backward arrow */
	  XD(pc.x, pc.y),	/* center_x (float) */
	  YD(pc.x, pc.y),	/* center_y (float) */
	  IROUND(XD(p0.x, p0.y)), /* 1st point user entered (p0) */
	  IROUND(YD(p0.x, p0.y)), 
	  IROUND(XD(pb.x, pb.y)), /* 2nd point user entered (bisection point)*/
	  IROUND(YD(pb.x, pb.y)),
	  IROUND(XD(p1.x, p1.y)), /* last point user entered (p1) */
	  IROUND(YD(p1.x, p1.y)));
  _update_buffer (&_plotter->outbuf);

  _plotter->drawstate->pos = p1; /* move to p1 (a libplot convention) */

  return 0;
}
