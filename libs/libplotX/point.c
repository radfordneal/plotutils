/* This file contains the point routine, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

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
  
  _evaluate_x_fgcolor();
  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
  XDrawPoint (_xdata.dpy, _xdata.window, _drawstate->gc, 
	      IROUND(XD(x,y)), IROUND(YD(x,y)));
  XDrawPoint (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
	      IROUND(XD(x,y)), IROUND(YD(x,y)));
				
  (_drawstate->pos).x = x;	/* move to the point */
  (_drawstate->pos).y = y;

  _handle_x_events();

  return 0;
}

int
point (x, y)
     int x, y;
{
  return fpoint ((double)x, (double)y);
}
