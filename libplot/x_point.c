/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_fpoint (double x, double y)
#else
_x_fpoint (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  _plotter->endpath(); /* flush polyline if any */
  
  /* select pen color as foreground color in GC used for drawing */
  _plotter->set_pen_color();

  if (_plotter->double_buffering)
	XDrawPoint (_plotter->dpy, _plotter->drawable3, 
		    _plotter->drawstate->gc_fg, 
		    IROUND(XD(x,y)), IROUND(YD(x,y)));
  else
    {
      if (_plotter->drawable1)
	XDrawPoint (_plotter->dpy, _plotter->drawable1, 
		    _plotter->drawstate->gc_fg, 
		    IROUND(XD(x,y)), IROUND(YD(x,y)));
      if (_plotter->drawable2)
	XDrawPoint (_plotter->dpy, _plotter->drawable2, 
		    _plotter->drawstate->gc_fg, 
		    IROUND(XD(x,y)), IROUND(YD(x,y)));
    }
				
  _plotter->drawstate->pos.x = x; /* move to the point */
  _plotter->drawstate->pos.y = y;

  _handle_x_events();

  return 0;
}
