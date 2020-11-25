/* This file contains the point routine, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y.

   In libplotfig a point is a single-vertex polyline (that is the only sort
   of point that xfig supports). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

int
fpoint (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fpoint() called when graphics device not open\n");
      return -1;
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
	  "#POLYLINE [OPEN]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d\n\t%d %d\n",
	  2,			/* polyline object */
	  P_OPEN,		/* polyline subtype */
	  _drawstate->fig_line_style, /* style */
	  _device_line_width(), /* thickness, in Fig display units */
	  _drawstate->fig_fgcolor, /* pen color */
	  _drawstate->fig_fillcolor, /* fill color */
	  _drawstate->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _drawstate->fig_fill_level, /* area fill */
	  _drawstate->fig_dash_length, /* style val, in Fig display units
					  (float) */
	  0,			/* join style = mitre */
	  0,			/* cap style = butt */
	  0,			/* radius (of arc boxes, ignored) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  1,			/* number of points in polyline */
	  IROUND(XD(x,y)), 
	  IROUND(YD(x,y))
	  );

  _update_buffer (&_outbuf);

  /* update our notion of position */
  (_drawstate->pos).x = x;
  (_drawstate->pos).y = y;

  return 0;
}

int
point (x, y)
     int x, y;
{
  return fpoint ((double)x, (double)y);
}
