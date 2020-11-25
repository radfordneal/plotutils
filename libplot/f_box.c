/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* Fig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

int
#ifdef _HAVE_PROTOS
_f_fbox (double x0, double y0, double x1, double y1)
#else
_f_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  int xd0, xd1, yd0, yd1;	/* in device coordinates */
  double xnew, ynew;

  if (!_plotter->open)
    {
      _plotter->error ("fbox: invalid operation");
      return -1;
    }

  /* if user coors -> device coors transformation does not preserve axis
     directions, use generic method to draw a closed polyline */

  if (!_plotter->drawstate->transform.axes_preserved)
    return _g_fbox (x0, y0, x1, y1);

  /* otherwise use xfig's box-drawing facility (which aligns boxes with the
     axes) */

  _plotter->endpath(); /* flush polyline if any */
  
  /* evaluate fig colors lazily, i.e. only when needed */
  _plotter->set_pen_color();
  _plotter->set_fill_color();
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_plotter->fig_last_priority >= POLYLINE_PRIORITY)
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;
  _plotter->fig_last_priority = POLYLINE_PRIORITY;

  sprintf(_plotter->outbuf.current,
	  "#POLYLINE [BOX]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d\n",
	  2,			/* polyline object */
	  P_BOX,		/* polyline subtype */
	  _fig_line_style[_plotter->drawstate->line_type], /* style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor,	/* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
		  /* style val, in Fig display units (float) */
	  _fig_dash_length[_plotter->drawstate->line_type], 
	  _fig_join_style[_plotter->drawstate->join_type], /* join style */
	  _fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
	  0,			/* radius (of arc boxes, ignored here) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  5			/* number of points in line */
	  );
  _update_buffer (&_plotter->outbuf);

  xd0 = IROUND(XD(x0, y0));
  yd0 = IROUND(YD(x0, y0));  
  xd1 = IROUND(XD(x1, y1));
  yd1 = IROUND(YD(x1, y1));  

  sprintf (_plotter->outbuf.current, "\t%d %d ", xd0, yd0);
  _update_buffer (&_plotter->outbuf);
  sprintf (_plotter->outbuf.current, "%d %d ", xd0, yd1);
  _update_buffer (&_plotter->outbuf);
  sprintf (_plotter->outbuf.current, "%d %d ", xd1, yd1);
  _update_buffer (&_plotter->outbuf);
  sprintf (_plotter->outbuf.current, "%d %d ", xd1, yd0);
  _update_buffer (&_plotter->outbuf);
  sprintf (_plotter->outbuf.current, "%d %d\n", xd0, yd0);
  _update_buffer (&_plotter->outbuf);

  /* move to center (libplot convention) */
  xnew = 0.5 * (x0 + x1);
  ynew = 0.5 * (y0 + y1);
  _plotter->drawstate->pos.x = xnew;
  _plotter->drawstate->pos.y = ynew;

  return 0;
}
