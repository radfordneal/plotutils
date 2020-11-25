/* This file contains the ellarc method, which is a GNU extension to
   libplot.  It draws an object: an arc of an ellipse, from p0=(x0,y0) to
   p1=(x1,y1).  The center of the ellipse will be at pc=(xc,yc).

   These conditions do not uniquely determine the elliptic arc (or the
   ellipse of which it is an arc).  We choose the elliptic arc so that it
   has control points p0, p1, and p0 + p1 - pc, where the third control
   point p0 + p1 - pc is simply the reflection of pc through the line
   determined by p0 and p1.  This means that the arc passes through p0 and
   p1, is tangent at p0 to the line segment joining p0 to p0 + p1 - pc, and
   is tangent at p1 to the line segment joining p1 to p0 + p1 - pc.  So it
   fits snugly into a triangle, the vertices of which are the three control
   points.
   
   This sort of elliptic arc is called a `quarter-ellipse', since it is an
   affinely transformed quarter-circle.  Specifically, it is an affinely
   transformed version of the first quadrant of a unit circle, with the
   affine transformation mapping (0,0) to pc, (0,1) to p0, (1,0) to p1, and
   (1,1) to the control point p0 + p1 - pc. */

/* In this version, we first check to see if the angle of inclination of
   the quarter-ellipse is zero, and the map from user coordinates to device
   coordinates preserves axes.  If both are true, we use native X rendering
   to draw the ellipse.  Otherwise we invoke the generic method, to
   approximate the elliptic arc by drawing an inscribed polyline. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define COLLINEAR(p0, p1, p2) \
	((p0.x * p1.y - p0.y * p1.x - p0.x * p2.y + \
	  p0.y * p2.x + p1.x * p2.y - p1.y * p2.x) == 0.0)

int
#ifdef _HAVE_PROTOS
_x_fellarc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_x_fellarc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point pc, p0, p1;

  if (!_plotter->open)
    {
      _plotter->error ("fellarc: invalid operation");
      return -1;
    }

  /* if affine map from user frame to device frame does not preserve
     coordinate axes, or X11's arc-drawing facility cannot be used, call
     the generic class method to draw an inscribed polyline */
  if (!(_plotter->drawstate->transform.axes_preserved 
	&& ((y0 == yc && x1 == xc) || (x0 == xc && y1 == yc))))
    return _g_fellarc (xc, yc, x0, y0, x1, y1);

  /* otherwise use X11's native arc-drawing facility, as follows */

  _plotter->endpath ();		/* flush polyline if any */

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  if (COLLINEAR (p0, p1, pc))
    /* collinear points, simply draw line segment from p0 to p1 */
    {
      _plotter->fmove (p0.x, p0.y);
      _plotter->fcont (p1.x, p1.y);
      return 0;
    }

    {				/* use native X arc rendering  */
      int x_orientation, y_orientation;
      int xorigin, yorigin;
      unsigned int squaresize_x, squaresize_y;
      int startangle, endangle, range;
      double rx, ry;

      /* axes flipped? (by default y-axis is, due to  X's flipped-y convention) */
      x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
      y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

      if (y0 == yc && x1 == xc)
	/* initial pt. on x-axis, final pt. on y-axis */
	{
	  /* semi-axes in user frame */
	  rx = (x0 > xc) ? x0 - xc : xc - x0;
	  ry = (y1 > yc) ? y1 - yc : yc - y1;
	  /* starting and ending angles; note flipped-y convention */
	  startangle = ((x0 > xc ? 1 : -1) * x_orientation == 1) ? 0 : 180;
	  endangle = ((y1 > yc ? 1 : -1) * y_orientation == -1) ? 90 : 270;
	}
      else
	/* initial pt. on y-axis, final pt. on x-axis */
	{	
	  /* semi-axes in user frame */
	  rx = (x1 > xc) ? x1 - xc : xc - x1;
	  ry = (y0 > yc) ? y0 - yc : yc - y0;
	  /* starting and ending angles; note flipped-y convention */
	  startangle = ((y0 > yc ? 1 : -1) * y_orientation == -1) ? 90 : 270;
	  endangle = ((x1 > xc ? 1 : -1) * x_orientation == 1) ? 0 : 180;
	}	  

      if (endangle < startangle)
	endangle += 360;
      range = endangle - startangle; /* always 90 or 270 */

      /* our convention: a quarter-ellipse can only be 90 degrees
	 of an X ellipse, not 270 degrees, so interchange points */
      if (range == 270)
	{
	  int tmp;

	  tmp = startangle;
	  startangle = endangle;
	  endangle = tmp;
	  range = 90;
	}
      
      if (startangle >= 360)
	/* avoid obscure X bug */
	startangle -= 360;	/* endangle no longer relevant */

      /* location of `origin' (upper left corner of bounding rect. on display)
	 and width and height; X's flipped-y convention affects these values */
      xorigin = IROUND(XD(xc - x_orientation * rx, 
			  yc - y_orientation * ry));
      yorigin = IROUND(YD(xc - x_orientation * rx, 
			  yc - y_orientation * ry));
      squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
      squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));
      
      /* sanity check */
      if (XOOB_INT(xorigin) || XOOB_INT(yorigin) || XOOB_UNSIGNED(squaresize_x)
	  || XOOB_UNSIGNED(squaresize_y)) return -1;

      /* should check for width, height being 0 here, and treat this
	 special case appropriately: FIXME */

      /* place current line attributes in GC's used for drawing and filling */
      _plotter->set_attributes();  

      if (_plotter->drawstate->fill_level)	/* not transparent */
	{
	  /* select fill color as foreground color in GC used for filling */
	  _plotter->set_fill_color();

	  if (_plotter->double_buffering)
	    XFillArc(_plotter->dpy, _plotter->drawable3, 
		     _plotter->drawstate->gc_fill, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     64 * startangle, 64 * range);
	  else
	    {
	      if (_plotter->drawable1)
		XFillArc(_plotter->dpy, _plotter->drawable1, 
			 _plotter->drawstate->gc_fill, 
			 xorigin, yorigin, squaresize_x, squaresize_y,
			 64 * startangle, 64 * range);
	      if (_plotter->drawable2)
		XFillArc(_plotter->dpy, _plotter->drawable2, 
			 _plotter->drawstate->gc_fill, 
			 xorigin, yorigin, squaresize_x, squaresize_y,
			 64 * startangle, 64 * range);
	    }
	}
      
      /* select pen color as foreground color in GC used for drawing */
      _plotter->set_pen_color();

      if (_plotter->double_buffering)
	XDrawArc(_plotter->dpy, _plotter->drawable3, 
		 _plotter->drawstate->gc_fg, 
		 xorigin, yorigin, squaresize_x, squaresize_y,
		 64 * startangle, 64 * range);
      else
	{
	  if (_plotter->drawable1)
	    XDrawArc(_plotter->dpy, _plotter->drawable1, 
		     _plotter->drawstate->gc_fg, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     64 * startangle, 64 * range);
	  if (_plotter->drawable2)
	    XDrawArc(_plotter->dpy, _plotter->drawable2, 
		     _plotter->drawstate->gc_fg, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     64 * startangle, 64 * range);
	}
    }

  _plotter->drawstate->pos = p1; /* move to p1 (a libplot convention) */

  _handle_x_events();

  return 0;
}

