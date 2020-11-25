/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

/* This version is for XDrawablePlotters (and XPlotters). */

#include "sys-defines.h"
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
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
      /* just move to center (a libplot convention), flushing path if any */
      _plotter->fmove (xc, yc);
      return 0;
    }
  
  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath ();	/* flush path if any */

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

  /* If ellipse must be drawn at an angle, or affine map from user frame to
     device frame does not preserve coordinate axes, call the generic class
     method to draw an inscribed polyline.  Specify that it's convex (to
     speed up rendering, if we're filling). */

  if (angle != 0.0 || !(_plotter->drawstate->transform.axes_preserved))
    {
      int retval;

      /* draw, move to center, flush out */
      retval = _g_fellipse (xc, yc, rx, ry, angle); 
      return retval;
    }

  /* otherwise use X11's native ellipse-drawing facility, as follows */

  rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure X problems */
  ry = (ry < 0.0 ? -ry : ry);  

  /* axes flipped? (by default y-axis is, due to X's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);
  
  /* location of `origin' (upper left corner of bounding rect. on
     display) and width and height; X's flipped-y convention affects
     these values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
  
  /* don't use zero dimensions if user specified nonzero */
  if (squaresize_x == 0 && rx > 0.0)
    squaresize_x = 1;
  if (squaresize_y == 0 && ry > 0.0)
    squaresize_y = 1;
  
  /* draw ellipse (elliptic arc, arc range = 64*360 64'ths of a degree),
     see x_endpath.c */
  _draw_elliptic_X_arc_internal (xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 0, 64 * 360);
  
  _plotter->drawstate->pos.x = xc; /* move to center (a libplot convention) */
  _plotter->drawstate->pos.y = yc;
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events();
  
  return 0;
}
