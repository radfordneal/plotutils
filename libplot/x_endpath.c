/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for XDrawablePlotters (and XPlotters).  By construction,
   for such Plotters our path storage buffer always includes either (1) a
   polyline, i.e., a sequence of line segments, or (2) a single circular or
   elliptic arc segment.  Those are the only two sorts of path that X11 can
   handle.  (For the latter to be added to the path buffer, the map from
   user to device coordinates must preserve axes.) */

/* If the line style is "solid" and the path has zero width, it has already
   been drawn; see x_cont.c.  So if the path doesn't need to be filled, we
   don't do anything.  If it does, we fill it, and then redraw it. */

/* When filling a polyline, we look at _plotter->drawstate->convex_path to
   determine which X rendering algorithm to use.  Our default algorithm is
   Complex (i.e. generic), but when drawing polygonal approximations to
   ellipses and rectangles, which we know must be convex, we use Convex to
   speed up rendering. */

/* Note: should insert some more X protocol out-of-bounds checks into this
   file via the XOOB_INT and XOOB_UNSIGNED macros.  FIXME. */

#include "sys-defines.h"
#include "extern.h"

/* number of XPoint structs we can store on the stack, for speed, without
   invoking malloc */
#define MAX_NUM_X_POINTS_ON_STACK 128

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

int
#ifdef _HAVE_PROTOS
_x_endpath (S___(Plotter *_plotter))
#else
_x_endpath (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool closed;
  int is_a_rectangle;
  int i, polyline_len;
  plPoint p0, p1, pc;
  XPoint *xarray, local_xarray[MAX_NUM_X_POINTS_ON_STACK];
  bool heap_storage;
  double xu_last, yu_last;
  bool identical_user_coordinates;
  /* The next few quantities are used only when drawing a filled circle,
     in the special subcase when our entire sequence of path vertices is
     mapped to a single X pixel.  That's the best we can do. */
  int sp_offset, xloc = 0, yloc = 0;
  unsigned int sp_size = 1;
  
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "endpath: invalid operation");
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

      /* use native X rendering to draw the (transformed) circular arc */
      _draw_elliptic_X_arc (R___(_plotter) p0, p1, pc);
  
      /* reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      /* maybe flush X output buffer and handle X events (a no-op for
	 XDrawablePlotters, which is overridden for XPlotters) */
      _maybe_handle_x_events (S___(_plotter));
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

      /* use native X rendering to draw the (transformed) elliptic arc */
      _draw_elliptic_X_arc_2 (R___(_plotter) p0, p1, pc);
  
      /* reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      /* maybe flush X output buffer and handle X events (a no-op for
	 XDrawablePlotters, which is overridden for XPlotters) */
      _maybe_handle_x_events (S___(_plotter));
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
      plPoint saved_pos;
      plGeneralizedPoint *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_points_in_path = _plotter->drawstate->points_in_path;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _plotter->savestate (S___(_plotter));
      _plotter->pentype (R___(_plotter) 1);
      _plotter->fillcolor (R___(_plotter)
			   _plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (R___(_plotter) 1);
      _plotter->linewidth (R___(_plotter) 0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_points_in_path - (closed ? 1 : 0); i++)
	/* draw each point as a filled circle, diameter = line width */
	_plotter->fcircle (R___(_plotter)
			   saved_datapoints[i].x, saved_datapoints[i].y, 
			   radius);
      _plotter->drawstate->points_are_connected = false;
      _plotter->restorestate (S___(_plotter));
      free (saved_datapoints);
      if (closed)
	_plotter->drawstate->pos = saved_pos; /* restore graphics cursor */
      return 0;
    }
  
  /* general case: points are vertices of a (non-disconnected) polyline */

  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* have something to draw */
    {
      /* Check for zero pen width, and a "solid" line style.  If so, and
	 this isn't a polygonalized built-in object (i.e. one of the convex
	 rectangles and ellipses that we can draw), we've already drawn the
	 polyline, segment by segment (see x_cont.c).  If there's no
	 filling to be done, that is; so in that case, we just reset the
	 buffer and return.  

	 If on the other hand there's filling to be done, we keep
	 going... which means that we'll fill the polyline, and then edge
	 it a second time.  There is a reason for this. */

      if (_plotter->drawstate->pen_type != 0 /* pen is present */
	  && _plotter->drawstate->line_type == L_SOLID
	  && !_plotter->drawstate->dash_array_in_effect
	  && _plotter->drawstate->quantized_device_line_width == 0
	  && !_plotter->drawstate->convex_path
	  && _plotter->drawstate->fill_type == 0)
	{
	  /* reset path storage buffer */
	  if (_plotter->drawstate->points_in_path > 0)
	    {
	      _plotter->drawstate->points_in_path = 0;
	      free (_plotter->drawstate->datapoints);
	      _plotter->drawstate->datapoints_len = 0;
	    }
	  /* maybe flush X output buffer and handle X events (a no-op for
	     XDrawablePlotters, which is overridden for XPlotters) */
	  _maybe_handle_x_events (S___(_plotter));
	  return 0;
	}

      /* could this path have been generated by a user invocation of box(),
	 i.e. does it define a rectangle traced counterclockwise from the
	 lower left corner? */

#define IS_A_RECTANGLE(len,p) \
((len) == 5 && (p)[0].x == (p)[4].x && (p)[0].y == (p)[4].y && \
(p)[0].x == (p)[3].x && (p)[1].x == (p)[2].x && \
(p)[0].y == (p)[1].y && (p)[2].y == (p)[3].y && \
(p)[0].x < (p)[1].x && (p)[0].y < (p)[2].y)  

      is_a_rectangle = IS_A_RECTANGLE(_plotter->drawstate->points_in_path,
				      _plotter->drawstate->datapoints);
  
      /* N.B.  If a rectangle, upper left corner = p[3], and also, width =
	 p[1].x - p[0].x and height = p[2].y - p[0].y (the latter
	 computation does _not_ use the flipped-y convention!) */

      /* prepare an array of XPoint structures (X11 uses short ints for
         these) */
      if (_plotter->drawstate->points_in_path <= MAX_NUM_X_POINTS_ON_STACK)
	/* store XPoints on stack, for speed */
	{
	  xarray = local_xarray;
	  heap_storage = false;
	}
      else
	/* store XPoints in heap */
	{
	  xarray = (XPoint *)_plot_xmalloc (_plotter->drawstate->points_in_path * sizeof(XPoint));
	  heap_storage = true;
	}

      /* convert vertices to device coordinates, removing runs; also keep
	 track of whether or not all points in user space are the same */

      xu_last = 0.0;
      yu_last = 0.0;
      identical_user_coordinates = true;

      polyline_len = 0;
      for (i = 0; i < _plotter->drawstate->points_in_path; i++)
	{
	  plGeneralizedPoint datapoint;
	  double xu, yu, xd, yd;
	  int device_x, device_y;

	  datapoint = _plotter->drawstate->datapoints[i];
	  xu = datapoint.x;
	  yu = datapoint.y;
	  xd = XD(xu, yu);
	  yd = YD(xu, yu);
	  device_x = IROUND(xd);
	  device_y = IROUND(yd);

	  if (i > 0 && (xu != xu_last || yu != yu_last))
	    /* in user space, not all points are the same */
	    identical_user_coordinates = false;	
      
	  if ((polyline_len == 0) 
	      || (device_x != xarray[polyline_len-1].x) 
	      || (device_y != xarray[polyline_len-1].y))
	    /* add point, in integer device coordinates, to the array */
	    {
	      xarray[polyline_len].x = device_x;
	      xarray[polyline_len].y = device_y;
	      polyline_len++;
	    }

	  xu_last = xu;
	  yu_last = yu;
	}

      /* compute the square size, and offset of upper left vertex from
	 center of square, that we'll use when handling the notorious
	 `special subcase' (see comments above and below) */

      if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
	{
	  sp_size = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
	  if (sp_size == 0) 
	    sp_size = 1;
	  sp_offset = (_plotter->drawstate->quantized_device_line_width + 1) / 2;
	  xloc = xarray[0].x - sp_offset;
	  yloc = xarray[0].y - sp_offset;
	}
      
      /* FIRST TASK: fill the polygon (if necessary). */

      if (_plotter->drawstate->fill_type) 
	/* not transparent, so fill the path */
	{
	  int x_polygon_type 
	    = (_plotter->drawstate->convex_path ? Convex : Complex);

	  if (_plotter->x_double_buffering != DBL_NONE)
	    {
	      if (!(_plotter->drawstate->points_in_path > 1 && polyline_len == 1))
		/* general subcase, not the abovementioned special subcase */
		{
		  /* update GC used for filling (specify GC by passing a
                     hint) */
		  _plotter->drawstate->x_gc_type = X_GC_FOR_FILLING;
		  _plotter->set_attributes (S___(_plotter));

		  /* select fill color as foreground color in GC used for
                     filling */
		  _plotter->set_fill_color (S___(_plotter));
		  
		  if (is_a_rectangle && polyline_len == 5)
		    /* call XFillRectangle, for speed */
		    XFillRectangle (_plotter->x_dpy, _plotter->x_drawable3,
				    _plotter->drawstate->x_gc_fill,
				    (int)(xarray[3].x), (int)(xarray[3].y),
				    (unsigned int)(xarray[1].x - xarray[0].x),
				    /* flipped y */
				    (unsigned int)(xarray[0].y - xarray[2].y));
		  else
		    /* can't call XFillRectangle */
		    XFillPolygon (_plotter->x_dpy, _plotter->x_drawable3,
				  _plotter->drawstate->x_gc_fill,
				  xarray, polyline_len,
				  x_polygon_type, CoordModeOrigin);
		}
	    }
	  else		/* not double buffering, no `x_drawable3' */
	    {
	      if (!(_plotter->drawstate->points_in_path > 1 && polyline_len == 1))
		/* general subcase, not the abovementioned special subcase */
		{
		  /* update GC used for filling (specify GC by passing a
                     hint) */
		  _plotter->drawstate->x_gc_type = X_GC_FOR_FILLING;
		  _plotter->set_attributes (S___(_plotter));

		  /* select fill color as foreground color in GC used for
                     filling */
		  _plotter->set_fill_color (S___(_plotter));

		  if (is_a_rectangle && polyline_len == 5)
		    /* call XFillRectangle, for speed */
		    {
		      if (_plotter->x_drawable1)
			XFillRectangle (_plotter->x_dpy, _plotter->x_drawable1, 
					_plotter->drawstate->x_gc_fill,
					(int)(xarray[3].x), (int)(xarray[3].y),
					(unsigned int)(xarray[1].x - xarray[0].x),
					/* flipped y */
					(unsigned int)(xarray[0].y - xarray[2].y));
		      if (_plotter->x_drawable2)
			XFillRectangle (_plotter->x_dpy, _plotter->x_drawable2, 
					_plotter->drawstate->x_gc_fill,
					(int)(xarray[3].x), (int)(xarray[3].y),
					(unsigned int)(xarray[1].x - xarray[0].x),
					/* flipped y */
					(unsigned int)(xarray[0].y - xarray[2].y));
		    }
		  else
		    /* can't call XFillRectangle */
		    {
		      if (_plotter->x_drawable1)
			XFillPolygon (_plotter->x_dpy, _plotter->x_drawable1, 
				      _plotter->drawstate->x_gc_fill,
				      xarray, polyline_len,
				      x_polygon_type, CoordModeOrigin);
		      if (_plotter->x_drawable2)
			XFillPolygon (_plotter->x_dpy, _plotter->x_drawable2, 
				      _plotter->drawstate->x_gc_fill,
				      xarray, polyline_len,
				      x_polygon_type, CoordModeOrigin);
		    }
		}
	    }
	}

      /* SECOND TASK: edge the polygon (if necessary). */

      if (_plotter->drawstate->pen_type) 
	/* pen is present, so edge the path */
	{
	  /* update GC used for drawing (specify GC by passing a hint) */
	  _plotter->drawstate->x_gc_type = X_GC_FOR_DRAWING;
	  _plotter->set_attributes (S___(_plotter));
      
	  /* select pen color as foreground color in GC used for drawing */
	  _plotter->set_pen_color (S___(_plotter));
      
	  /* Check first for the notorious special subcase: all user-space
	     points in the polyline were mapped to a single integer X
	     pixel.  If (1) they weren't all the same to begin with, or (2)
	     they were all the same to begin with and the cap mode is
	     "round", then draw as a filled circle of diameter equal to the
	     line width; otherwise draw nothing. */
      
	  if (_plotter->x_double_buffering != DBL_NONE)
	    /* double buffering, have a `x_drawable3' to draw into */
	    {
	      if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
		/* special subcase */
		{
		  if (identical_user_coordinates == false
		      || _plotter->drawstate->cap_type == CAP_ROUND)
		    {
		      if (sp_size == 1) /* why why oh why? */
			XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3, 
				    _plotter->drawstate->x_gc_fg, 
				    xarray[0].x, xarray[0].y);
		      else
			XFillArc(_plotter->x_dpy, _plotter->x_drawable3,
				 _plotter->drawstate->x_gc_fg, 
				 xloc, yloc, sp_size, sp_size,
				 0, 64 * 360);
		    }
		}
	      else
		/* general subcase */
		/* NOTE: this code is what libplot uses to draw nearly all
		   polylines, in the case when double buffering is used */
		{
		  if (is_a_rectangle && polyline_len == 5 
		      && _plotter->drawstate->line_type == L_SOLID)
		    /* call XDrawRectangle, for speed */
		    {
		      XDrawRectangle (_plotter->x_dpy, _plotter->x_drawable3, 
				      _plotter->drawstate->x_gc_fg, 
				      (int)(xarray[3].x), (int)(xarray[3].y),
				      (unsigned int)(xarray[1].x - xarray[0].x),
				      /* flipped y */
				      (unsigned int)(xarray[0].y - xarray[2].y));
		    }
		  else
		    /* can't call XDrawRectangle */
		    {
		      XDrawLines (_plotter->x_dpy, _plotter->x_drawable3, 
				  _plotter->drawstate->x_gc_fg, 
				  xarray, polyline_len,
				  CoordModeOrigin);
		    }
		}
	    }
	  else
	    /* not double buffering, have no `x_drawable3' */
	    {
	      if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
		/* special subcase */
		{
		  if (identical_user_coordinates == false
		      || _plotter->drawstate->cap_type == CAP_ROUND)
		    {
		      if (sp_size == 1) /* why why oh why? */
			{
			  if (_plotter->x_drawable1)
			    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1,
					_plotter->drawstate->x_gc_fg, 
					xarray[0].x, xarray[0].y);
			  if (_plotter->x_drawable2)
			    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2,
					_plotter->drawstate->x_gc_fg, 
					xarray[0].x, xarray[0].y);
			}
		      else
			{
			  if (_plotter->x_drawable1)
			    XFillArc(_plotter->x_dpy, _plotter->x_drawable1,
				     _plotter->drawstate->x_gc_fg, 
				     xloc, yloc, sp_size, sp_size,
				     0, 64 * 360);
			  if (_plotter->x_drawable2)
			    XFillArc(_plotter->x_dpy, _plotter->x_drawable2,
				     _plotter->drawstate->x_gc_fg, 
				     xloc, yloc, sp_size, sp_size,
				     0, 64 * 360);
			}
		    }
		}
	      else
		/* general subcase */
		/* NOTE: this code is what libplot uses to draw nearly all
		   polylines; at least, if double buffering is not used */
		{
		  if (is_a_rectangle && polyline_len == 5 
		      && _plotter->drawstate->line_type == L_SOLID)
		    /* call XDrawRectangle, for speed */
		    {
		      if (_plotter->x_drawable1)
			XDrawRectangle (_plotter->x_dpy, _plotter->x_drawable1, 
					_plotter->drawstate->x_gc_fg, 
					(int)(xarray[3].x), (int)(xarray[3].y),
					(unsigned int)(xarray[1].x - xarray[0].x),
					/* flipped y */
					(unsigned int)(xarray[0].y - xarray[2].y));
		      if (_plotter->x_drawable2)
			XDrawRectangle (_plotter->x_dpy, _plotter->x_drawable2, 
					_plotter->drawstate->x_gc_fg, 
					(int)(xarray[3].x), (int)(xarray[3].y),
					(unsigned int)(xarray[1].x - xarray[0].x),
					/* flipped y */
					(unsigned int)(xarray[0].y - xarray[2].y));
		    }
		  else
		    /* can't use XDrawRectangle() */
		    {
		      if (_plotter->x_drawable1)
			XDrawLines (_plotter->x_dpy, _plotter->x_drawable1, 
				    _plotter->drawstate->x_gc_fg, 
				    xarray, polyline_len,
				    CoordModeOrigin);
		      if (_plotter->x_drawable2)
			XDrawLines (_plotter->x_dpy, _plotter->x_drawable2, 
				    _plotter->drawstate->x_gc_fg, 
				    xarray, polyline_len,
				    CoordModeOrigin);
		    }
		}
	    }
	}
  
      /* reset path storage buffer */
      if (_plotter->drawstate->points_in_path > 0)
	{
	  if (heap_storage)
	    free (xarray);		/* free malloc'd array of XPoints */
	}
    }
  
  /* reset path storage buffer */
  if (_plotter->drawstate->points_in_path > 0)
    {
      _plotter->drawstate->points_in_path = 0;
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
    }

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));
  return 0;
}

/* Use native X rendering to draw what would be a circular arc in the user
   frame on an X display.  If this is called, the map from user to device
   coordinates is assumed to preserve coordinate axes (it may be
   anisotropic [x and y directions scaled differently], and it may include
   a reflection through either or both axes).  So it will be a circular or
   elliptic arc in the device frame, of the sort that X11 supports. */

void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
#else
_draw_elliptic_X_arc (R___(_plotter) p0, p1, pc)
     S___(Plotter *_plotter;)
     plPoint p0, p1, pc;
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

  _draw_elliptic_X_arc_internal (R___(_plotter)
				 xorigin, yorigin, 
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
void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc_2 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
#else
_draw_elliptic_X_arc_2 (R___(_plotter) p0, p1, pc)
     S___(Plotter *_plotter;)
     plPoint p0, p1, pc;
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

  _draw_elliptic_X_arc_internal (R___(_plotter)
				 xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
  return;
}
