/* This file contains the endpoly() routine, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated calls to the cont() routine.  (See the comments in cont.c.)
   The construction may be terminated, and the polyline object finalized,
   by an explict call to endpoly().

   If endpoly() is called when no polyline is under construction, it
   has no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* whether or not to suppress the flushing out of unfilled polylines when
   they've reached a length MAX_UNFILLED_POLYLINE_LENGTH */

Boolean _suppress_polyline_flushout = FALSE;

int
endpoly ()
{
  int i;
  Boolean closed;
  XPoint *adhoc_array;
  int polyline_len;
    
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: endpoly() called when graphics device not open\n");
      return -1;
    }

  if (_drawstate->PointsInLine == 0)	/* nothing to do */
    return 0;
  
  if (_drawstate->PointsInLine == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
      free (_drawstate->datapoints);
      _drawstate->datapoints_len = 0;
      _drawstate->PointsInLine = 0;

      return 0;
    }
  
  if ((_drawstate->PointsInLine >= 3) /* check for closure */
      && (_drawstate->datapoints[_drawstate->PointsInLine - 1].x == _drawstate->datapoints[0].x)
      && (_drawstate->datapoints[_drawstate->PointsInLine - 1].y == _drawstate->datapoints[0].y))
    closed = TRUE;
  else
    closed = FALSE;		/* 2-point ones should be open */
  
  /* must prepare an array of XPoint structures (X11 uses short ints for these) */
  adhoc_array = (XPoint *)_plot_xmalloc (_drawstate->PointsInLine * sizeof(XPoint));

  /* convert vertices to device coordinates, removing runs */
  polyline_len = 0;
  for (i = 0; i < _drawstate->PointsInLine; i++)
    {
      int device_x, device_y;

      device_x = IROUND(XD(_drawstate->datapoints[i].x, 
			   _drawstate->datapoints[i].y));
      device_y = IROUND(YD(_drawstate->datapoints[i].x, 
			   _drawstate->datapoints[i].y));

      if ((polyline_len == 0) || (device_x != adhoc_array[polyline_len-1].x) || (device_y != adhoc_array[polyline_len-1].y))
	{
	  adhoc_array[polyline_len].x = device_x;
	  adhoc_array[polyline_len].y = device_y;
	  polyline_len++;
	}
    }

  if (_drawstate->points_are_connected)
    {
    /* general case: points are vertices of a polyline */

      if (_drawstate->fill_level)	/* not transparent */
	{
	  _evaluate_x_fillcolor();
	  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fillcolor);
	  XFillPolygon (_xdata.dpy, _xdata.window, _drawstate->gc, 
			adhoc_array, polyline_len,
			Complex, CoordModeOrigin);
	  XFillPolygon (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
			adhoc_array, polyline_len,
			Complex, CoordModeOrigin);
	}

      _evaluate_x_fgcolor();
      XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
      XDrawLines (_xdata.dpy, _xdata.window, _drawstate->gc, 
		  adhoc_array, polyline_len,
		  CoordModeOrigin);
      XDrawLines (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		  adhoc_array, polyline_len,
		  CoordModeOrigin);
    }
  else
    {
      /* special case: disconnected points, no real polyline */
      _evaluate_x_fgcolor();
      XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
      XDrawPoints (_xdata.dpy, _xdata.window, _drawstate->gc, 
		   adhoc_array, polyline_len,
		   CoordModeOrigin);
      XDrawPoints (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		   adhoc_array, polyline_len,
		   CoordModeOrigin);
    }
  
  /* reset polyline storage buffer */
  if (_drawstate->PointsInLine > 0)
    {
      free (adhoc_array);

      _drawstate->PointsInLine = 0;
      free (_drawstate->datapoints);
      _drawstate->datapoints_len = 0;
    }

  _handle_x_events();
  return 0;
}
