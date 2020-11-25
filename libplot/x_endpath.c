/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated invocations of the cont() method.  (See the comments in
   g_cont.c.)  The construction may be terminated, and the polyline object
   finalized, by an explict invocation of endpath().

   If endpath() is invoked when no polyline is under construction, it has
   no effect. */

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
  
  /* must prepare an array of XPoint structures (X11 uses short ints for these) */
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

      if ((polyline_len == 0) || (device_x != xarray[polyline_len-1].x) || (device_y != xarray[polyline_len-1].y))
	{
	  xarray[polyline_len].x = device_x;
	  xarray[polyline_len].y = device_y;
	  polyline_len++;
	}
    }

  /* place current line attributes in the X GC */
  _plotter->set_attributes();  

  if (_plotter->drawstate->points_are_connected)
    {
    /* general case: points are vertices of a polyline */

      if (_plotter->drawstate->fill_level)	/* not transparent */
	{
	  /* select fill color as X foreground color */
	  _plotter->set_fill_color();

	  if (_plotter->drawable1)
	    XFillPolygon (_plotter->dpy, _plotter->drawable1, 
			  _plotter->drawstate->gc, 
			  xarray, polyline_len,
			  Complex, CoordModeOrigin);
	  if (_plotter->drawable2)
	    XFillPolygon (_plotter->dpy, _plotter->drawable2, 
			  _plotter->drawstate->gc, 
			  xarray, polyline_len,
			  Complex, CoordModeOrigin);
	}

      /* select pen color as X foreground color */
      _plotter->set_pen_color();

      if (_plotter->drawable1)
	XDrawLines (_plotter->dpy, _plotter->drawable1, 
		    _plotter->drawstate->gc, 
		    xarray, polyline_len,
		    CoordModeOrigin);
      if (_plotter->drawable2)
	XDrawLines (_plotter->dpy, _plotter->drawable2, 
		    _plotter->drawstate->gc, 
		    xarray, polyline_len,
		    CoordModeOrigin);
    }
  else
    {
      /* special case: disconnected points, no real polyline */

      /* select pen color as X foreground color */
      _plotter->set_pen_color();

      if (_plotter->drawable1)
	XDrawPoints (_plotter->dpy, _plotter->drawable1, _plotter->drawstate->gc, 
		     xarray, polyline_len,
		     CoordModeOrigin);
      if (_plotter->drawable2)
	XDrawPoints (_plotter->dpy, _plotter->drawable2, _plotter->drawstate->gc, 
		     xarray, polyline_len,
		     CoordModeOrigin);
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
