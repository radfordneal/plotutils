/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* This version is for XDrawablePlotters and XPlotters.  It calls
   _maybe_handle_x_events(), which is a no-op for the former but not the
   latter (it flushes the X output buffer and may also check for events).
   Since point() is used mostly by people drawing images, it may be invoked
   a great many times.  To speed things up, the call to
   _maybe_handle_x_events() is performed only once per X_POINT_FLUSH_PERIOD
   invocations of this function. */

#define X_POINT_FLUSH_PERIOD 8

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_fpoint (R___(Plotter *_plotter) double x, double y)
#else
_x_fpoint (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  int ix, iy;
  plColor oldcolor, newcolor;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fpoint: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */
  
  /* set pen color as foreground color in GC used for drawing (but first,
     check whether we can avoid a function call) */
  newcolor = _plotter->drawstate->fgcolor;
  oldcolor = _plotter->drawstate->x_current_fgcolor; /* i.e. as stored in gc */
  if (newcolor.red != oldcolor.red 
      || newcolor.green != oldcolor.green 
      || newcolor.blue != oldcolor.blue
      || ! _plotter->drawstate->x_gc_fgcolor_status)
  _plotter->set_pen_color (S___(_plotter));

  ix = IROUND(XD(x,y));
  iy = IROUND(YD(x,y));  
  if (_plotter->x_double_buffering != DBL_NONE)
	/* double buffering, have a `x_drawable3' to draw into */
	XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3, 
		    _plotter->drawstate->x_gc_fg, 
		    ix, iy);
  else
    /* not double buffering, have no `x_drawable3' */
    {
      if (_plotter->x_drawable1)
	XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1, 
		    _plotter->drawstate->x_gc_fg, 
		    ix, iy);
      if (_plotter->x_drawable2)
	XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2, 
		    _plotter->drawstate->x_gc_fg, 
		    ix, iy);
    }
				
  _plotter->drawstate->pos.x = x; /* move to the point */
  _plotter->drawstate->pos.y = y;

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  if (_plotter->x_paint_pixel_count % X_POINT_FLUSH_PERIOD == 0)
    _maybe_handle_x_events (S___(_plotter));
  _plotter->x_paint_pixel_count++;

  return 0;
}
