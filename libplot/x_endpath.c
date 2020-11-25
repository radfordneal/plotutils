/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated invocations of the cont() method.  (See the comments in
   g_cont.c.)  The construction may be terminated, and the polyline object
   finalized, by an explict invocation of endpath().

   If endpath() is invoked when no polyline is under construction, it has
   no effect. */

/* Note: should insert some X protocol out-of-bounds checks into this file
   via the XOOB_INT and XOOB_UNSIGNED macros.  FIXME. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_endpath (void)
#else
_x_endpath ()
#endif
{
  int i;
  bool closed;
  XPoint *xarray;
  int polyline_len;
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

  /* If a circular arc has been stashed rather than drawn, force it to be
     drawn by invoking farc() with the `immediate' flag set.  Note that
     if an arc is stashed, PointsInLine must be zero. */
  if (_plotter->drawstate->arc_stashed) 
    { 
      double axc = _plotter->drawstate->axc;
      double ayc = _plotter->drawstate->ayc;
      double ax0 = _plotter->drawstate->ax0;
      double ay0 = _plotter->drawstate->ay0;
      double ax1 = _plotter->drawstate->ax1;
      double ay1 = _plotter->drawstate->ay1;

      _plotter->drawstate->arc_immediate = true; 
      _plotter->drawstate->arc_polygonal = false; /* advisory only */
      _plotter->farc (axc, ayc, ax0, ay0, ax1, ay1);
      _plotter->drawstate->arc_immediate = false;
      _plotter->drawstate->arc_stashed = false;
    }

  if (_plotter->drawstate->PointsInLine == 0)	/* nothing to do */
    return 0;
  
  if (_plotter->drawstate->PointsInLine == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;

      return 0;
    }
  
  if ((_plotter->drawstate->PointsInLine >= 3) /* check for closure */
      && (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1].x == _plotter->drawstate->datapoints[0].x)
      && (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1].y == _plotter->drawstate->datapoints[0].y))
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
      Point *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_PointsInLine = _plotter->drawstate->PointsInLine;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;

      _plotter->savestate();
      _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (1);
      _plotter->linewidth (0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_PointsInLine - (closed ? 1 : 0); i++)
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
  xarray = (XPoint *)_plot_xmalloc (_plotter->drawstate->PointsInLine * sizeof(XPoint));

  /* convert vertices to device coordinates, removing runs */
  polyline_len = 0;
  for (i = 0; i < _plotter->drawstate->PointsInLine; i++)
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
     subcase' (see above and below) */

  if (_plotter->drawstate->PointsInLine > 1 && polyline_len == 1)
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
	  if (!(_plotter->drawstate->PointsInLine > 1 && polyline_len == 1))
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
	  if (!(_plotter->drawstate->PointsInLine > 1 && polyline_len == 1))
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
  
  /* We check first for the special subcase: more than one point in the
     polyline, but they were all mapped to a single integer X pixel.
     In this case we draw the path as a filled circle, of diameter
     equal to the line width.  Provided that the cap mode isn't "butt",
	 that is; if it is, we don't draw anything. */
  
  if (_plotter->double_buffering != DBL_NONE)
    /* double buffering, have a `drawable3' to draw into */
    {
      if (_plotter->drawstate->PointsInLine > 1 && polyline_len == 1)
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
      if (_plotter->drawstate->PointsInLine > 1 && polyline_len == 1)
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
  
  /* reset polyline storage buffer */
  if (_plotter->drawstate->PointsInLine > 0)
    {
      free (xarray);		/* free our array of XPoints */

      _plotter->drawstate->PointsInLine = 0;
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
    }
  
  _handle_x_events();
  return 0;
}
