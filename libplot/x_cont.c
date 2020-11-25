/* This file contains the cont() method, which is a standard part of the
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths.  By repeatedly
   invoking cont(), the user may construct a polyline of arbitrary length.
   arc() and ellarc() may also be invoked, to add circular or elliptic arc
   elements to the path.  The path will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          e.g. by invoking move(), linemod(), linewidth(), pencolor(), 
	  fillcolor(), or filltype(), or 
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
_x_fcont (R___(Plotter *_plotter) double x, double y)
#else
_x_fcont (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  bool something_drawn = false;
  int retval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fcont: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->pen_type != 0 /* pen is present */
      && _plotter->drawstate->line_type == L_SOLID
      && !_plotter->drawstate->dash_array_in_effect
      && _plotter->drawstate->points_are_connected
      && _plotter->drawstate->quantized_device_line_width == 0
      && !_plotter->drawstate->convex_path
      && _plotter->drawstate->fill_type == 0)
    /* zero-width solid line, and we're not drawing this line segment as
       part of a polygonalized built-in object (i.e. the convex rectangles
       and ellipses that we can draw): if there's no filling to be done,
       we `pre-draw', i.e., draw the line segment immediately */
    {
      /* use same variables for points #1 and #2, since reusing them works
	 around an obscure bug in gcc 2.7.2.3 that rears its head if -O2 is
	 used */
      double xu, yu, xd, yd;
      int x1, y1, x2, y2;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress; this line segment begins a new one */
	{
	  /* update GC used for drawing (specify GC by passing a hint) */
	  _plotter->drawstate->x_gc_type = X_GC_FOR_DRAWING;
	  _plotter->set_attributes (S___(_plotter));
	  
	  /* select pen color as foreground color in GC used for drawing */
	  _plotter->set_pen_color (S___(_plotter));
	}
  
      /* starting and ending points for zero-width line */
      xu = _plotter->drawstate->pos.x;
      yu = _plotter->drawstate->pos.y;
      xd = XD(xu, yu);
      yd = YD(xu, yu);
      x1 = IROUND(xd);
      y1 = IROUND(yd);
      xd = XD(x,y);
      yd = YD(x,y);
      x2 = IROUND(xd);
      y2 = IROUND(yd);

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
      else
	/* line segment in terms of integer device coordinates has zero
	   length; but if it has nonzero length in user coordinates, draw
	   it as a single pixel unless cap type is "butt" */
	if (!(_plotter->drawstate->cap_type == CAP_BUTT
	      && xu == x && yu == y))
	  {
	    if (_plotter->x_double_buffering != DBL_NONE)
	      /* double buffering, have a `x_drawable3' to draw into */
	      XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3,
			  _plotter->drawstate->x_gc_fg, 
			  x1, y1);
	    else
	      /* not double buffering */
	      {
		if (_plotter->x_drawable1)
		  XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1,
			      _plotter->drawstate->x_gc_fg, 
			      x1, y1);
		if (_plotter->x_drawable2)
		  XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2,
			      _plotter->drawstate->x_gc_fg, 
			      x1, y1);
	      }
	  }
    }

  /* invoke generic method */
  retval = _g_fcont (R___(_plotter) x, y);

  if (something_drawn)
    /* maybe flush X output buffer and handle X events (a no-op for
       XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return retval;
}
