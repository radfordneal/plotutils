/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_fbox (double x0, double y0, double x1, double y1)
#else
_x_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  unsigned int width, height;
  int xdmin, xdmax, ydmin, ydmax;
  int xd0, xd1, yd0, yd1;
  double xnew, ynew;

  if (!_plotter->open)
    {
      _plotter->error ("fbox: invalid operation");
      return -1;
    }

  /* if user coors -> device coors transformation does not preserve axis
     directions, use generic method rather than using the native X
     rectangle-drawing facility (which aligns boxes with the axes) */
  if (!_plotter->drawstate->transform.axes_preserved)
    return _g_fbox (x0, y0, x1, y1);

  _plotter->endpath (); /* flush polyline if any */

  xd0 = IROUND(XD (x0, y0));
  yd0 = IROUND(YD (x0, y0));  
  xd1 = IROUND(XD (x1, y1));
  yd1 = IROUND(YD (x1, y1));  

  xdmin = IMIN (xd0, xd1);
  ydmin = IMIN (yd0, yd1);
  xdmax = IMAX (xd0, xd1);
  ydmax = IMAX (yd0, yd1);

  width = (unsigned int)(xdmax - xdmin);
  height = (unsigned int)(ydmax - ydmin);
  
  if (XOOB_INT(xdmin) || XOOB_INT(ydmin) 
      || XOOB_UNSIGNED(width) || XOOB_UNSIGNED(height))
    return -1;

  /* should check for width, height being 0 here, and treat this special
     case appropriately: FIXME */

  /* Semantics of XDrawRectangle() and XFillRectangle() are as follows.  To
     draw a 10x10 rectangle, the boundary of the rectangular region
     [0,9]x[0,9], XDrawRectangle() should be called with arguments 0,0,9,9.
     But to paint each pixel in [0,9]x[0,9], XFillRectangle() should be
     called with args 0,0,10,10.

     We don't bother incrementing the args of XFillRectangle, because we
     follow our XFillRectangle() by an XDrawRectangle(). */

  /* place current line attributes in X GC's used for drawing and filling */
  _plotter->set_attributes();  

  if (_plotter->drawstate->fill_level)	/* not transparent */
    {
      /* select fill color as foreground color in GC used for filling */
      _plotter->set_fill_color();

      if (_plotter->double_buffering != DBL_NONE)
	XFillRectangle (_plotter->dpy, _plotter->drawable3,
			_plotter->drawstate->gc_fill, 
			xdmin, ydmin, width, height);
      else
	{
	  if (_plotter->drawable1)
	    XFillRectangle (_plotter->dpy, _plotter->drawable1, 
			    _plotter->drawstate->gc_fill, 
			    xdmin, ydmin, width, height);
	  if (_plotter->drawable2)
	    XFillRectangle (_plotter->dpy, _plotter->drawable2, 
			    _plotter->drawstate->gc_fill, 
			    xdmin, ydmin, width, height);
	}
    }

  /* select pen color as foreground color in GC used for drawing */
  _plotter->set_pen_color();

  if (_plotter->double_buffering != DBL_NONE)
    XDrawRectangle (_plotter->dpy, _plotter->drawable3,
		    _plotter->drawstate->gc_fg, 
		    xdmin, ydmin, width, height);
  else
    {
      if (_plotter->drawable1)
	XDrawRectangle (_plotter->dpy, _plotter->drawable1, 
			_plotter->drawstate->gc_fg, 
			xdmin, ydmin, width, height);
      if (_plotter->drawable2)
	XDrawRectangle (_plotter->dpy, _plotter->drawable2, 
			_plotter->drawstate->gc_fg, 
			xdmin, ydmin, width, height);
    }

  /* move to center (libplot convention) */
  xnew = 0.5 * (x0 + x1);
  ynew = 0.5 * (y0 + y1);
  _plotter->drawstate->pos.x = xnew;
  _plotter->drawstate->pos.y = ynew;

  _handle_x_events();

  return 0;
}
