/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* In this version, we first check to see if the angle of inclination is
   zero, and the map from the user frame to the device frame preserves
   axes.  If these are both true, we can use native X rendering to draw the
   ellipse.  If they are not, we draw an inscribed polyline, by invoking
   the generic method. */

int
#ifdef _HAVE_PROTOS
_x_fellipse (double xc, double yc, double rx, double ry, double angle)
#else
_x_fellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
#endif
{
  int ninetymult = IROUND(angle / 90.0);

  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

 /* if angle is multiple of 90 degrees, modify to permit use of native X
    arc rendering */
  if (angle == (double) (90 * ninetymult))
    {
      angle = 0.0;
      if (ninetymult % 2)
	{
	  double temp;
	  
	  temp = rx;
	  rx = ry;
	  ry = temp;
	}
    }

  /* if ellipse must be drawn at an angle, or affine map from user frame to
     device frame does not preserve coordinate axes, call the generic class
     method to draw an inscribed polyline */
  if (angle != 0.0 || !(_plotter->drawstate->transform.axes_preserved))
    return _g_fellipse (xc, yc, rx, ry, angle);

  /* otherwise use X11's native ellipse-drawing facility, as follows */

  _plotter->endpath (); /* flush polyline if any */

  rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure X problems */
  ry = (ry < 0.0 ? -ry : ry);  

  {				/* use native X ellipse rendering  */
    int x_orientation, y_orientation;
    int xorigin, yorigin;
    unsigned int squaresize_x, squaresize_y;
    
    /* axes flipped? (by default y-axis is, due to  X's flipped-y convention) */
    x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
    y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; X's flipped-y convention affects these values */
    xorigin = IROUND(XD(xc - x_orientation * rx, 
			yc - y_orientation * ry));
    yorigin = IROUND(YD(xc - x_orientation * rx, 
			yc - y_orientation * ry));
    squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
    squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
    
    /* sanity check */
    if (XOOB_INT(xorigin) || XOOB_INT(yorigin) || XOOB_UNSIGNED(squaresize_x)
	|| XOOB_UNSIGNED(squaresize_y)) return -1;

    /* don't use zero dimensions if user specified nonzero */
    if (squaresize_x == 0 && rx > 0.0)
      squaresize_x = 1;
    if (squaresize_y == 0 && ry > 0.0)
      squaresize_y = 1;
    
    /* place current line attributes in GC's used for drawing and filling */
    _plotter->set_attributes();  

    if (_plotter->drawstate->fill_level) /* not transparent */
      {
	/* select fill color as foreground color in GC used for filling */
	_plotter->set_fill_color();

	if (_plotter->double_buffering)
	  XFillArc(_plotter->dpy, _plotter->drawable3,
		   _plotter->drawstate->gc_fill, 
		   xorigin, yorigin, squaresize_x, squaresize_y, 0, 64 * 360);
	else
	  {
	    if (_plotter->drawable1)
	      XFillArc(_plotter->dpy, _plotter->drawable1, 
		       _plotter->drawstate->gc_fill, 
		       xorigin, yorigin, squaresize_x, squaresize_y, 0, 64 * 360);
	    if (_plotter->drawable2)
	      XFillArc(_plotter->dpy, _plotter->drawable2, 
		       _plotter->drawstate->gc_fill, 
		       xorigin, yorigin, squaresize_x, squaresize_y, 0, 64 * 360);
	  }
      }
    
    /* select pen color as foreground color in GC used for drawing */
    _plotter->set_pen_color();

    if (_plotter->double_buffering)
      XDrawArc(_plotter->dpy, _plotter->drawable3, 
	       _plotter->drawstate->gc_fg,
	       xorigin, yorigin, squaresize_x, squaresize_y, 0, 64 * 360);
    else
      {
	if (_plotter->drawable1)
	  XDrawArc(_plotter->dpy, _plotter->drawable1, 
		   _plotter->drawstate->gc_fg,
		   xorigin, yorigin, squaresize_x, squaresize_y, 0, 64 * 360);
	if (_plotter->drawable2)
	  XDrawArc(_plotter->dpy, _plotter->drawable2, 
		   _plotter->drawstate->gc_fg,
		   xorigin, yorigin, squaresize_x, squaresize_y, 0, 64 * 360);
      }
  }
  
  _plotter->drawstate->pos.x = xc; /* move to center (a libplot convention) */
  _plotter->drawstate->pos.y = yc;
  
  _handle_x_events();
  
  return 0;
}
