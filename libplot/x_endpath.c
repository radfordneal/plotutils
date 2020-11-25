/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for XPlotters.  By construction, for XPlotters our path
   storage buffer always includes either (1) a sequence of line segments,
   or (2) a single circular or elliptic arc segment.  Those are the only
   two sorts of path that X11 can handle.  (For the latter to be included,
   the map from user to device coordinates must preserve axes.) */

/* Note: should insert some more X protocol out-of-bounds checks into this
   file via the XOOB_INT and XOOB_UNSIGNED macros.  FIXME. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

/* forward references */
static void _draw_elliptic_X_arc __P((Point p0, Point p1, Point pc));
static void _draw_elliptic_X_arc_2 __P((Point p0, Point p1, Point pc));

int
#ifdef _HAVE_PROTOS
_x_endpath (void)
#else
_x_endpath ()
#endif
{
  bool closed;
  int i, polyline_len;
  Point p0, p1, pc;
  XPoint *xarray;
  /* The next few quantities are used only when drawing a filled circle,
     in the special subcase when our entire sequence of path vertices is
     mapped to a single X pixel.  That's the best we can do. */
  int sp_offset, xloc = 0, yloc = 0;
  unsigned int sp_size = 1;
      
  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path == 2
      && _plotter->drawstate->datapoints[1].type == S_ARC)
    /* path buffer contains a single circular arc, not a polyline */
    {
      double x0 = _plotter->drawstate->datapoints[0].x;
      double y0 = _plotter->drawstate->datapoints[0].y;      
      double x1 = _plotter->drawstate->datapoints[1].x;
      double y1 = _plotter->drawstate->datapoints[1].y;      
      double xc = _plotter->drawstate->datapoints[1].xc;
      double yc = _plotter->drawstate->datapoints[1].yc;      

      pc.x = xc, pc.y = yc;
      p0.x = x0, p0.y = y0;
      p1.x = x1, p1.y = y1;

      /* use native X rendering to draw the (transformed) circular arc, via
         the routine below */
      _draw_elliptic_X_arc (p0, p1, pc);
  
      /* reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _handle_x_events();
      return 0;
    }

  if (_plotter->drawstate->points_in_path == 2
      && _plotter->drawstate->datapoints[1].type == S_ELLARC)
    /* path buffer contains a single elliptic arc, not a polyline */
    {
      double x0 = _plotter->drawstate->datapoints[0].x;
      double y0 = _plotter->drawstate->datapoints[0].y;      
      double x1 = _plotter->drawstate->datapoints[1].x;
      double y1 = _plotter->drawstate->datapoints[1].y;      
      double xc = _plotter->drawstate->datapoints[1].xc;
      double yc = _plotter->drawstate->datapoints[1].yc;      

      pc.x = xc, pc.y = yc;
      p0.x = x0, p0.y = y0;
      p1.x = x1, p1.y = y1;

      /* use native X rendering to draw the (transformed) elliptic arc, via
         the routine below */
      _draw_elliptic_X_arc_2 (p0, p1, pc);
  
      /* reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _handle_x_events();
      return 0;
    }

  /* neither of above applied, so path buffer contains a polyline, not an
     arc */

  if (_plotter->drawstate->points_in_path == 0)	/* nothing to do */
    return 0;
  
  if (_plotter->drawstate->points_in_path == 1)	/* shouldn't happen */
    {
      /* just reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      return 0;
    }
  
  if ((_plotter->drawstate->points_in_path >= 3) /* check for closure */
      && (_plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].x == _plotter->drawstate->datapoints[0].x)
      && (_plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].y == _plotter->drawstate->datapoints[0].y))
    closed = true;
  else
    closed = false;		/* 2-point ones should be open */
  
  /* Special case: disconnected points, no real polyline.  We switch to a
     temporary datapoints buffer for this.  This is a hack, needed because
     the fcircle() method calls endpath(), which would otherwise mess the
     real databuffer up. */
  if (!_plotter->drawstate->points_are_connected)
    {
      Point saved_pos;
      GeneralizedPoint *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_points_in_path = _plotter->drawstate->points_in_path;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _plotter->savestate();
      _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (1);
      _plotter->linewidth (0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_points_in_path - (closed ? 1 : 0); i++)
	/* draw each point as a filled circle, diameter = line width */
	_plotter->fcircle (saved_datapoints[i].x, saved_datapoints[i].y, 
			   radius);
      _plotter->drawstate->points_are_connected = false;
      _plotter->restorestate();
      free (saved_datapoints);
      if (closed)
	_plotter->drawstate->pos = saved_pos; /* restore graphics cursor */
      return 0;
    }
  
  /* general case: points are vertices of a polyline */

  /* prepare an array of XPoint structures (X11 uses short ints for these) */
  xarray = (XPoint *)_plot_xmalloc (_plotter->drawstate->points_in_path * sizeof(XPoint));

  /* convert vertices to device coordinates, removing runs */
  polyline_len = 0;
  for (i = 0; i < _plotter->drawstate->points_in_path; i++)
    {
      int device_x, device_y;

      device_x = IROUND(XD(_plotter->drawstate->datapoints[i].x, 
			   _plotter->drawstate->datapoints[i].y));
      device_y = IROUND(YD(_plotter->drawstate->datapoints[i].x, 
			   _plotter->drawstate->datapoints[i].y));

      if ((polyline_len == 0) 
	  || (device_x != xarray[polyline_len-1].x) 
	  || (device_y != xarray[polyline_len-1].y))
	{
	  xarray[polyline_len].x = device_x;
	  xarray[polyline_len].y = device_y;
	  polyline_len++;
	}
    }

  /* compute the square size, and offset of upper left vertex from center
     of square, that we'll use when handling the notorious `special
     subcase' (see comments above and below) */

  if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
    {
      sp_size = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
      if (sp_size == 0) 
	sp_size = 1;
      sp_offset = (_plotter->drawstate->quantized_device_line_width + 1) / 2;
      xloc = xarray[0].x - sp_offset;
      yloc = xarray[0].y - sp_offset;
    }
      
  /* place current line attributes in GC's used for drawing and filling */
  _plotter->set_attributes();  
  
  if (_plotter->drawstate->fill_level) 
    /* not transparent, must fill */
    {
      if (_plotter->double_buffering != DBL_NONE)
	{
	  if (!(_plotter->drawstate->points_in_path > 1 && polyline_len == 1))
	    /* general subcase, not the abovementioned special subcase */
	    {
	      /* select fill color as foreground color in GC used for filling*/
	      _plotter->set_fill_color();
		  
	      XFillPolygon (_plotter->dpy, _plotter->drawable3,
			    _plotter->drawstate->gc_fill,
			    xarray, polyline_len,
			    Complex, CoordModeOrigin);
	    }
	}
      else		/* not double buffering, no `drawable3' */
	{
	  if (!(_plotter->drawstate->points_in_path > 1 && polyline_len == 1))
	    /* general subcase, not the abovementioned special subcase */
	    {
	      /* select fill color as foreground color in GC used for filling*/
	      _plotter->set_fill_color();

	      if (_plotter->drawable1)
		XFillPolygon (_plotter->dpy, _plotter->drawable1, 
			      _plotter->drawstate->gc_fill,
			      xarray, polyline_len,
			      Complex, CoordModeOrigin);
	      if (_plotter->drawable2)
		XFillPolygon (_plotter->dpy, _plotter->drawable2, 
			      _plotter->drawstate->gc_fill,
			      xarray, polyline_len,
			      Complex, CoordModeOrigin);
	    }
	}
    }

  /* select pen color as foreground color in GC used for drawing */
  _plotter->set_pen_color();
  
  /* We check first for the notorious special subcase: more than one point
     in the polyline, but they were all mapped to a single integer X pixel.
     In this case we draw the path as a filled circle, of diameter equal to
     the line width.  Provided that the cap mode isn't "butt", that is; if
     it is, we don't draw anything. */
  
  if (_plotter->double_buffering != DBL_NONE)
    /* double buffering, have a `drawable3' to draw into */
    {
      if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
	/* special subcase */
	{
	  if (_plotter->drawstate->cap_type != CAP_BUTT)
	    {
	      if (sp_size == 1) /* why why oh why? */
		XDrawPoint (_plotter->dpy, _plotter->drawable3, 
			    _plotter->drawstate->gc_fg, 
			    xarray[0].x, xarray[0].y);
	      else
		XFillArc(_plotter->dpy, _plotter->drawable3,
			 _plotter->drawstate->gc_fg, 
			 xloc, yloc, sp_size, sp_size,
			 0, 64 * 360);
	    }
	}
      else
	/* general subcase */
	XDrawLines (_plotter->dpy, _plotter->drawable3, 
		    _plotter->drawstate->gc_fg, 
		    xarray, polyline_len,
		    CoordModeOrigin);
    }
  else
    /* not double buffering, have no `drawable3' */
    {
      if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
	/* special subcase */
	{
	  if (_plotter->drawstate->cap_type != CAP_BUTT)
	    {
	      if (sp_size == 1) /* why why oh why? */
		{
		  XDrawPoint (_plotter->dpy, _plotter->drawable1,
			      _plotter->drawstate->gc_fg, 
			      xarray[0].x, xarray[0].y);
		  XDrawPoint (_plotter->dpy, _plotter->drawable2,
			      _plotter->drawstate->gc_fg, 
			      xarray[0].x, xarray[0].y);
		}
	      else
		{
		  if (_plotter->drawable1)
		    XFillArc(_plotter->dpy, _plotter->drawable1,
			     _plotter->drawstate->gc_fg, 
			     xloc, yloc, sp_size, sp_size,
			     0, 64 * 360);
		  if (_plotter->drawable2)
		    XFillArc(_plotter->dpy, _plotter->drawable2,
			     _plotter->drawstate->gc_fg, 
			     xloc, yloc, sp_size, sp_size,
			     0, 64 * 360);
		}
	    }
	}
      else
	/* general subcase */
	{
	  if (_plotter->drawable1)
	    XDrawLines (_plotter->dpy, _plotter->drawable1, 
			_plotter->drawstate->gc_fg, 
			xarray, polyline_len,
			CoordModeOrigin);
	  if (_plotter->drawable2)
	    XDrawLines (_plotter->dpy, _plotter->drawable2, 
			_plotter->drawstate->gc_fg, 
			xarray, polyline_len,
			CoordModeOrigin);
	}
    }
  
  /* reset path storage buffer */
  if (_plotter->drawstate->points_in_path > 0)
    {
      free (xarray);		/* free our array of XPoints */

      _plotter->drawstate->points_in_path = 0;
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
    }
  
  _handle_x_events();
  return 0;
}

/* Use native X rendering to draw what would be a circular arc in the user
   frame on an X display.  If this is called, the map from user to device
   coordinates is assumed to preserve coordinate axes (it may be
   anisotropic [x and y directions scaled differently], and it may include
   a reflection through either or both axes).  So it will be a circular or
   elliptic arc in the device frame, of the sort that the X11 drawing
   protocol supports. */
static void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc (Point p0, Point p1, Point pc)
#else
_draw_elliptic_X_arc (p0, p1, pc)
     Point p0, p1, pc;
#endif
{
  double radius;
  double theta0, theta1;
  int startangle, anglerange;
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

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

  _draw_elliptic_X_arc_internal (xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
  return;
}

/* Use native X rendering to draw what would be a quarter-ellipse in the
   user frame on an X display.  If this is called, the map from user to
   device coordinates is assumed to preserve coordinate axes (it may be
   anisotropic [x and y directions scaled differently], and it may include
   a reflection through either or both axes).  So it will be a
   quarter-ellipse in the device frame, of the sort that the X11 drawing
   protocol supports. */
static void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc_2 (Point p0, Point p1, Point pc)
#else
_draw_elliptic_X_arc_2 (p0, p1, pc)
     Point p0, p1, pc;
#endif
{
  double rx, ry;
  double x0, y0, x1, y1, xc, yc;
  int startangle, endangle, anglerange;
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  /* axes flipped? (by default y-axis is, due to  X's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  xc = pc.x, yc = pc.y;
  x0 = p0.x, y0 = p0.y;
  x1 = p1.x, y1 = p1.y;

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
  anglerange = endangle - startangle; /* always 90 or 270 */

  /* our convention: a quarter-ellipse can only be 90 degrees
     of an X ellipse, not 270 degrees, so interchange points */
  if (anglerange == 270)
    {
      int tmp;

      tmp = startangle;
      startangle = endangle;
      endangle = tmp;
      anglerange = 90;
    }
      
  if (startangle >= 360)
    /* avoid obscure X bug */
    startangle -= 360;		/* endangle no longer relevant */

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; X's flipped-y convention affects these values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));
      
  /* reexpress in 64'ths of a degree (X11 convention) */
  startangle *= 64;
  anglerange *= 64;

  _draw_elliptic_X_arc_internal (xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
  return;
}

/* Internal routine, uses native X rendering to draw an elliptic arc on an
   X display.  Takes account of the possible account of more than one
   drawable, and the possible need for filling.  This is invoked by the
   routines above, and when drawing ellipses (see x_ellipse.c). */
void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc_internal (int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
#else
_draw_elliptic_X_arc_internal (xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange)
     int xorigin, yorigin; 
     unsigned int squaresize_x, squaresize_y; 
     int startangle, anglerange;
#endif
{
  /* sanity check */
  if (XOOB_INT(xorigin) || XOOB_INT(yorigin) || XOOB_UNSIGNED(squaresize_x)
      || XOOB_UNSIGNED(squaresize_y)) return;

  /* should check for width, height being 0 here, and treat this special
     case appropriately?  FIXME. */

  /* place current line attributes in GC's used for drawing and filling */
  _plotter->set_attributes();  

  if (_plotter->drawstate->fill_level) /* not transparent */
    {
      /* select fill color as foreground color in GC used for filling */
      _plotter->set_fill_color();

      if (_plotter->double_buffering != DBL_NONE)
	XFillArc(_plotter->dpy, _plotter->drawable3, 
		 _plotter->drawstate->gc_fill, 
		 xorigin, yorigin, squaresize_x, squaresize_y,
		 startangle, anglerange);
      else
	{
	  if (_plotter->drawable1)
	    XFillArc(_plotter->dpy, _plotter->drawable1, 
		     _plotter->drawstate->gc_fill, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	  if (_plotter->drawable2)
	    XFillArc(_plotter->dpy, _plotter->drawable2, 
		     _plotter->drawstate->gc_fill, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	}
    }
      
  /* select pen color as foreground color in GC used for drawing */
  _plotter->set_pen_color();

  if (_plotter->double_buffering != DBL_NONE)
    XDrawArc(_plotter->dpy, _plotter->drawable3, 
	     _plotter->drawstate->gc_fg, 
	     xorigin, yorigin, squaresize_x, squaresize_y,
	     startangle, anglerange);
  else
    {
      if (_plotter->drawable1)
	XDrawArc(_plotter->dpy, _plotter->drawable1, 
		 _plotter->drawstate->gc_fg, 
		 xorigin, yorigin, squaresize_x, squaresize_y,
		 startangle, anglerange);
      if (_plotter->drawable2)
	XDrawArc(_plotter->dpy, _plotter->drawable2, 
		 _plotter->drawstate->gc_fg, 
		 xorigin, yorigin, squaresize_x, squaresize_y,
		 startangle, anglerange);
    }
  
  return;
}
