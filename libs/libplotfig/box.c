/* This file is the box routine, which is a standard part of libplot.  It
   draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* Fig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

int
fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fbox() called when graphics device not open\n");
      return -1;
    }

  /* if user coors -> device coors transformation does not preserve axis
     directions, draw closed polyline rather than using xfig's native
     box-drawing facility (which aligns boxes with the axes) */
  if (!_drawstate->transform.axes_preserved)
    {
      fmove (x0, y0);
      fcont (x0, y1);
      fcont (x1, y1);
      fcont (x1, y0);
      fcont (x0, y0);
      fmove (x1, y1);
      return 0;
    }

  endpoly();			/* flush polyline if any */
  
  /* evaluate fig colors lazily, i.e. only when needed */
  _evaluate_fig_fgcolor();
  _evaluate_fig_fillcolor();
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_drawstate->fig_last_priority > POLYLINE_PRIORITY)
    if (_drawstate->fig_drawing_depth > 0)
      (_drawstate->fig_drawing_depth)--;
  _drawstate->fig_last_priority = POLYLINE_PRIORITY;

  sprintf(_outbuf.current,
	  "#POLYLINE [BOX]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d\n",
	  2,			/* polyline object */
	  P_BOX,		/* polyline subtype */
	  _drawstate->fig_line_style, /* style */
	  _device_line_width(), /* thickness, in Fig display units */
	  _drawstate->fig_fgcolor,	/* pen color */
	  _drawstate->fig_fillcolor, /* fill color */
	  _drawstate->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _drawstate->fig_fill_level, /* area fill */
	  _drawstate->fig_dash_length, /* style val, in Fig display units
					   (float) */
	  _drawstate->fig_join_style, /* join style */
	  _drawstate->fig_cap_style, /* cap style */
	  0,			/* radius (of arc boxes, ignored here) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  5			/* number of points in line */
	  );
  _update_buffer (&_outbuf);

  sprintf (_outbuf.current, "\t%d %d ", IROUND(XD(x0,y0)), IROUND(YD(x0,y0)));
  _update_buffer (&_outbuf);
  sprintf (_outbuf.current, "%d %d ", IROUND(XD(x0,y0)), IROUND(YD(x1,y1)));
  _update_buffer (&_outbuf);
  sprintf (_outbuf.current, "%d %d ", IROUND(XD(x1,y1)), IROUND(YD(x1,y1)));
  _update_buffer (&_outbuf);
  sprintf (_outbuf.current, "%d %d ", IROUND(XD(x1,y1)), IROUND(YD(x0,y0)));
  _update_buffer (&_outbuf);
  sprintf (_outbuf.current, "%d %d\n", IROUND(XD(x0,y0)), IROUND(YD(x0,y0)));
  _update_buffer (&_outbuf);

  (_drawstate->pos).x = x1;	/* move to final corner */
  (_drawstate->pos).y = y1;

  return 0;
}

int
box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  return fbox ((double)x0, (double)y0, (double)x1, (double)y1);
}
