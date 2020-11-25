/* This file contains the cont() method, which is a standard part of the
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths.  By repeatedly
   invoking cont(), the user may construct a polyline of arbitrary length.
   arc() and ellarc() may also be invoked, to add circular or elliptic arc
   elements to the path.  The path will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by invoking move(), linemod(), linewidth(), color(), fillcolor(),
          or filltype(), or 
     (3) draws some non-path object, by invoking box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) invokes restorestate() to restore an earlier drawing state. */

/* This version is for XDrawable and X Plotters.  It will draw line
   segments even before any of (1)--(4) occurs, if the line width is zero
   and the line style is "solid".  See comments in x_endpath.c regarding
   this.

   This hack makes it possible, after doing `graph -TX -W0', to type in
   points manually, and see the corresponding polyline drawn in real time.
   The `-x' and `-y' options must of course be specified too, to set the
   axis limits in advance. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_fcont (double x, double y)
#else
_x_fcont (x, y)
     double x, y;
#endif
{
  bool something_drawn = false;
  int retval;

  if (!_plotter->open)
    {
      _plotter->error ("fcont: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->line_type == L_SOLID
      && !_plotter->drawstate->dash_array_in_effect
      && _plotter->drawstate->points_are_connected
      && _plotter->drawstate->quantized_device_line_width == 0)
    /* zero-width solid line: draw line segment immediately */
    {
      double xu1, yu1, xd1, yd1, xd2, yd2;
      int x1, y1, x2, y2;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress; this line segment begins a new one */
	{
	  /* place line attributes in GC's used for drawing and filling */
	  _plotter->set_attributes();  
	  
	  /* select pen color as foreground color in GC used for drawing */
	  _plotter->set_pen_color();
	}
  
      /* starting and ending points for zero-width line */
      xu1 = _plotter->drawstate->pos.x;
      yu1 = _plotter->drawstate->pos.y;
      xd1 = XD(xu1, yu1);
      yd1 = YD(xu1, yu1);
      xd2 = XD(x,y);
      yd2 = YD(x,y);
      x1 = IROUND(xd1);
      y1 = IROUND(yd1);
      x2 = IROUND(xd2);
      y2 = IROUND(yd2);

      if (x1 != x2 || y1 != y2)
	/* line segment has nonzero length, so draw it */
	{
	  if (_plotter->x_double_buffering != DBL_NONE)
	    /* double buffering, have a `x_drawable3' to draw into */
	    XDrawLine (_plotter->x_dpy, _plotter->x_drawable3, 
		       _plotter->drawstate->x_gc_fg, x1, y1, x2, y2);
	  else
	    {
	      if (_plotter->x_drawable1)
		XDrawLine (_plotter->x_dpy, _plotter->x_drawable1, 
			   _plotter->drawstate->x_gc_fg, x1, y1, x2, y2);
	      if (_plotter->x_drawable2)
		XDrawLine (_plotter->x_dpy, _plotter->x_drawable2, 
			   _plotter->drawstate->x_gc_fg, x1, y1, x2, y2);
	    }

	  something_drawn = true;
	}
    }

  /* invoke generic method */
  retval = _g_fcont (x, y);

  if (something_drawn)
    /* maybe flush X output buffer and handle X events (a no-op for
       XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events();

  return retval;
}
