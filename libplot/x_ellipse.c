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
_x_fellipse (R___(Plotter *_plotter) double xc, double yc, double rx, double ry, double angle)
#else
_x_fellipse (R___(_plotter) xc, yc, rx, ry, angle)
     S___(Plotter *_plotter;)
     double xc, yc, rx, ry, angle;
#endif
{
  int ninetymult = IROUND(angle / 90.0);
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fellipse: invalid operation");
      return -1;
    }

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
      /* just move to center (a libplot convention), flushing path if any */
      _plotter->fmove (R___(_plotter) xc, yc);
      return 0;
    }
  
  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter));	/* flush path if any */

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

  rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure X11 problems */
  ry = (ry < 0.0 ? -ry : ry);  

  /* axes flipped? (by default y-axis is, due to X's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);
  
  /* location of `origin' (upper left corner of bounding rect. for ellipse)
     and width and height; X's flipped-y convention affects these values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
  
  /* If ellipse must be drawn at an angle, or affine map from user frame to
     device frame does not preserve coordinate axes, call the generic class
     method to draw an inscribed polyline.

     Do the same if squaresize_{x|y} is rounded down to zero, which is a
     case that XFillArc() and XDrawArc() can't handle (see below). */

  if (angle != 0.0 || !(_plotter->drawstate->transform.axes_preserved)
      || squaresize_x == 0 || squaresize_y == 0)
    {
      int retval;

      /* draw, move to center, flush out */
      retval = _g_fellipse (R___(_plotter) xc, yc, rx, ry, angle); 
      return retval;
    }

  /* otherwise use X11's native ellipse-drawing facility, as follows */

  /* draw ellipse (elliptic arc aligned with the coordinate axes, arc range
     = 64*360 64'ths of a degree); see below */
  _draw_elliptic_X_arc_internal (R___(_plotter) 
				 xorigin, yorigin,
				 squaresize_x, squaresize_y, 
				 0, 64 * 360);
  
  _plotter->drawstate->pos.x = xc; /* move to center (a libplot convention) */
  _plotter->drawstate->pos.y = yc;
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));
  
  return 0;
}

/* Use native X rendering to draw an elliptic arc on an X display.  Takes
   account of the possible presence of more than one drawable, and the
   possible need for filling.

   Invoked by the above, and in x_endpath.c.

   Only the case squaresize_x > 0 and squaresize_y > 0 is handled, since
   XFillArc() and XDrawArc() do not support squaresize_x = 0 or
   squaresize_y = 0. */

void
#ifdef _HAVE_PROTOS
_draw_elliptic_X_arc_internal (R___(Plotter *_plotter) int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
#else
_draw_elliptic_X_arc_internal (R___(_plotter) xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange)
     S___(Plotter *_plotter;)
     int xorigin, yorigin; 
     unsigned int squaresize_x, squaresize_y; 
     int startangle, anglerange;
#endif
{
  /* sanity check */
  if (squaresize_x == 0 || squaresize_y == 0)
    return;

  /* sanity check */
  if (XOOB_INT(xorigin) || XOOB_INT(yorigin) || XOOB_UNSIGNED(squaresize_x)
      || XOOB_UNSIGNED(squaresize_y)) return;

  if (_plotter->drawstate->fill_type)
    /* not transparent, so fill the arc */
    {
      /* update GC used for filling (specify GC by passing a hint) */
      _plotter->drawstate->x_gc_type = X_GC_FOR_FILLING;
      _plotter->set_attributes (S___(_plotter));

      /* select fill color as foreground color in GC used for filling */
      _plotter->set_fill_color (S___(_plotter));

      if (_plotter->x_double_buffering != DBL_NONE)
	XFillArc(_plotter->x_dpy, _plotter->x_drawable3, 
		 _plotter->drawstate->x_gc_fill, 
		 xorigin, yorigin, squaresize_x, squaresize_y,
		 startangle, anglerange);
      else
	{
	  if (_plotter->x_drawable1)
	    XFillArc(_plotter->x_dpy, _plotter->x_drawable1, 
		     _plotter->drawstate->x_gc_fill, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	  if (_plotter->x_drawable2)
	    XFillArc(_plotter->x_dpy, _plotter->x_drawable2, 
		     _plotter->drawstate->x_gc_fill, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	}
    }
  
  if (_plotter->drawstate->pen_type)
    /* pen is present, so edge the arc */
    {
      /* update GC used for drawing (specify GC by passing a hint) */
      _plotter->drawstate->x_gc_type = X_GC_FOR_DRAWING;
      _plotter->set_attributes (S___(_plotter));
      
      /* select pen color as foreground color in GC used for drawing */
      _plotter->set_pen_color (S___(_plotter));
      
      /* edge the arc */
      if (_plotter->x_double_buffering != DBL_NONE)
	XDrawArc(_plotter->x_dpy, _plotter->x_drawable3, 
		 _plotter->drawstate->x_gc_fg, 
		 xorigin, yorigin, squaresize_x, squaresize_y,
		 startangle, anglerange);
      else
	{
	  if (_plotter->x_drawable1)
	    XDrawArc(_plotter->x_dpy, _plotter->x_drawable1, 
		     _plotter->drawstate->x_gc_fg, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	  if (_plotter->x_drawable2)
	    XDrawArc(_plotter->x_dpy, _plotter->x_drawable2, 
		     _plotter->drawstate->x_gc_fg, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	}
    }
  
  return;
}
