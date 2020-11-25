/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y.

   So far as a FigPlotter objects goes, a point is a single-vertex polyline
   (that is the only sort of point that xfig supports). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

int
#ifdef _HAVE_PROTOS
_f_fpoint (double x, double y)
#else
_f_fpoint (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

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
	  "#POLYLINE [OPEN]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d\n\t%d %d\n",
	  2,			/* polyline object */
	  P_OPEN,		/* polyline subtype */
	  _fig_line_style[_plotter->drawstate->line_type], /* style */
	  1,			/* thickness, in Fig display units */
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fgcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  20,			/* fig fill level (20 = full intensity) */
		  /* style val, in Fig display units (float) */
	  _fig_dash_length[_plotter->drawstate->line_type], 
	  FIG_JOIN_ROUND,	/* join style = round */
	  FIG_CAP_ROUND,	/* cap style = round */
	  0,			/* radius (of arc boxes, ignored) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  1,			/* number of points in polyline */
	  IROUND(XD(x,y)), 
	  IROUND(YD(x,y))
	  );

  _update_buffer (&_plotter->outbuf);

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  return 0;
}
