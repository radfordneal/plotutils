/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does. */

/* If the transformation from user to device coordinates preserves
   coordinate axes, the circular arc will be mapped onto an elliptic arc,
   the axes of the ellipse being aligned with the axes.  In this case we
   call _draw_elliptic_X_arc() to use the native X ellipse drawing
   facility.  Otherwise, we invoke the generic arc-drawing method to draw
   an inscribed polyline. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

/* forward references */
static void _draw_elliptic_X_arc __P((Point p0, Point p1, Point pc));

int
#ifdef _HAVE_PROTOS
_x_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_x_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point pc, p0, p1;

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
  if (_plotter->drawstate->arc_stashed
      && !_plotter->drawstate->arc_immediate)
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
       don't actually draw the arc, just stash it for later drawing. */
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
     user frame to the device frame preserves axes. */

  /* if affine map from user frame to device frame does not preserve axes,
     draw a polygonal approximation by invoking fcont() repeatedly */
  if (!_plotter->drawstate->transform.axes_preserved
      || (_plotter->drawstate->arc_immediate
	  && _plotter->drawstate->arc_polygonal)
      || _plotter->drawstate->PointsInLine > 0)
    {
      _plotter->drawstate->arc_stashed = false;	/* avoid infinite recursion */
      _draw_circular_arc (p0, p1, pc);
      return 0;
    }

  /* otherwise use X11's native arc-drawing facility, as follows */

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

  /* use native X rendering via the routine below */
  _draw_elliptic_X_arc (p0, p1, pc);
  
  _plotter->drawstate->pos = p1;	/* move to p1 (a libplot convention) */

  _handle_x_events();
  return 0;
}

/* At the time this function is called, pc is assumed to have been adjusted
   if necessary, to lie on the perpendicular bisector between p0, p1.  If
   this is called, the map from user to device coordinates must preserve
   coordinate axes (it may be anisotropic [x and y directions scaled
   differently], or include a reflection through either or both axes,
   however). */
static void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc (Point p0, Point p1, Point pc)
#else
_draw_elliptic_X_arc (p0, p1, pc)
     Point p0, p1, pc;
#endif
{
  double theta0, theta1;
  double radius;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;
  int startangle, anglerange;
  int x_orientation, y_orientation;

  /* axes flipped? (by default y-axis is, due to  X's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  /* radius of circular arc in user frame is distance to p0, and also to p1 */
  radius = DIST(pc, p0);

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; X's flipped-y convention affects these values */
  xorigin = IROUND(XD(pc.x - x_orientation * radius, 
		      pc.y - y_orientation * radius));
  yorigin = IROUND(YD(pc.x - x_orientation * radius, 
		      pc.y - y_orientation * radius));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * radius, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * radius));

  /* sanity check */
  if (XOOB_INT(xorigin) || XOOB_INT(yorigin) || XOOB_UNSIGNED(squaresize_x)
      || XOOB_UNSIGNED(squaresize_y)) return;

  /* should check for width, height being 0 here, and treat this special
     case appropriately: FIXME */

  theta0 = _xatan2 (-y_orientation * (p0.y - pc.y), 
		    x_orientation * (p0.x - pc.x)) / M_PI;
  theta1 = _xatan2 (-y_orientation * (p1.y - pc.y), 
		    x_orientation * (p1.x - pc.x)) / M_PI;

  if (theta1 < theta0)
    theta1 += 2.0;		/* adjust so that difference > 0 */
  if (theta0 < 0.0)
    {
      theta0 += 2.0;		/* adjust so that startangle > 0 */
      theta1 += 2.0;
    }

  if (theta1 - theta0 > 1.0)	/* swap if angle appear to be > 180 degrees */
    {
      double tmp;
      
      tmp = theta0;
      theta0 = theta1;
      theta1 = tmp;
      theta1 += 2.0;		/* adjust so that difference > 0 */      
    }

  if (theta0 >= 2.0 && theta1 >= 2.0)
    /* avoid obscure X bug */
    {
      theta0 -= 2.0;
      theta1 -= 2.0;
    }

  startangle = IROUND(64 * theta0 * 180.0); /* in 64'ths of a degree */
  anglerange = IROUND(64 * (theta1 - theta0) * 180.0); /* likewise */

  /* place current line attributes in the X GC's */
  _plotter->set_attributes();  

  if (_plotter->drawstate->fill_level)	/* not transparent */
    {
      /* select fill color as foreground color in GC used for filling */
      _plotter->set_fill_color();

      if (_plotter->double_buffering)
	XFillArc (_plotter->dpy, _plotter->drawable3, 
		  _plotter->drawstate->gc_fill, 
		  xorigin, yorigin, squaresize_x, squaresize_y, 
		  startangle, anglerange);
      else
	{
	  if (_plotter->drawable1)
	    XFillArc (_plotter->dpy, _plotter->drawable1, 
		      _plotter->drawstate->gc_fill, 
		      xorigin, yorigin, squaresize_x, 
		      squaresize_y, startangle, anglerange);
	  if (_plotter->drawable2)
	    XFillArc (_plotter->dpy, _plotter->drawable2, 
		      _plotter->drawstate->gc_fill, 
		      xorigin, yorigin, squaresize_x, squaresize_y, 
		      startangle, anglerange);
	}
    }

  /* select pen color as foreground color in GC used for drawing */
  _plotter->set_pen_color();

  if (_plotter->double_buffering)
  if (_plotter->drawable1)
    XDrawArc (_plotter->dpy, _plotter->drawable3, 
	      _plotter->drawstate->gc_fg, 
	      xorigin, yorigin, squaresize_x, squaresize_y, 
	      startangle, anglerange);
  else
    {
      if (_plotter->drawable1)
	XDrawArc (_plotter->dpy, _plotter->drawable1, 
		  _plotter->drawstate->gc_fg, 
		  xorigin, yorigin, squaresize_x, squaresize_y, 
		  startangle, anglerange);
      if (_plotter->drawable2)
	XDrawArc (_plotter->dpy, _plotter->drawable2, 
		  _plotter->drawstate->gc_fg, 
		  xorigin, yorigin, squaresize_x, squaresize_y, 
		  startangle, anglerange);
    }
      
  return;
}
