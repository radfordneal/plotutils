/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. 

   This implementation is for XDrawablePlotters.  It supports one or two
   drawables, which must be associated with the same display and have the
   same dimensions (width, height, depth).  A `drawable' is either a window
   or a pixmap. */

/* This file also contains the internal functions _x_maybe_get_new_colormap
   and _x_maybe_handle_x_events, which are no-ops.  However, they are
   virtual and are overridden in the derived XPlotter class, which both
   attempts to switch to a private colormap when color cells run out, and
   processes its own X events. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_openpl (void)
#else
_x_openpl ()
#endif
{
  Window root1, root2;
  int x, y;
  unsigned int border_width, depth1, depth2;
  unsigned int width1, height1, width2, height2;
  const char *bg_color_name_s;
  unsigned int width, height, depth;
  const char *double_buffer_s;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  if (_plotter->x_dpy == (Display *)NULL)
    /* pathological: user didn't set XDRAWABLE_DISPLAY parameter */
    {
      _plotter->error ("can't open Plotter, XDRAWABLE_DISPLAY parameter is null");
      return -1;
    }

  /* find out how long polylines can get on this X display */
  _plotter->hard_polyline_length_limit = 
    XMaxRequestSize(_plotter->x_dpy) / 2;

  /* determine dimensions of drawable(s) */
  if (_plotter->x_drawable1)
    XGetGeometry (_plotter->x_dpy, _plotter->x_drawable1,
		  &root1, &x, &y, &width1, &height1, &border_width, &depth1);
  if (_plotter->x_drawable2)
    XGetGeometry (_plotter->x_dpy, _plotter->x_drawable2,
		  &root2, &x, &y, &width2, &height2, &border_width, &depth2);
  
  if (_plotter->x_drawable1 && _plotter->x_drawable2)
    /* sanity check */
    {
      if (width1 != width2 || height1 != height2 
	  || depth1 != depth2 || root1 != root2)
	{
	  _plotter->error("can't open Plotter, X drawables have unequal parameters");
	  return -1;
	}
    }
  
  if (_plotter->x_drawable1)
    {
      width = width1;
      height = height1;
      depth = depth1;
    }
  else if (_plotter->x_drawable2)
    {
      width = width2;
      height = height2;
      depth = depth1;
    }
  else
  /* both are NULL, and we won't really be drawing, so these are irrelevant */
    {
      width = 1;
      height = 1;
      depth = 1;
    }

  _plotter->imin = 0;
  _plotter->imax = width - 1;
  /* note flipped-y convention for this device: for j, min > max */
  _plotter->jmin = height - 1;
  _plotter->jmax = 0;
  
  /* Invoke generic method; among other things, this invokes savestate() to
     create a drawing state (see x_savestate.c).  The drawing state won't
     be ready for drawing graphics, since it won't contain an X font or a
     meaningful line width.  To retrieve an X font and set the line width,
     the user will need to invoke space() after openpl(). */

  /* We don't allow openpl() to reset the frame number to zero (in
     XDrawable Plotters that is, unlike XPlotters).  Incrementing the frame
     number monotonically facilitates color cell management, since we've
     only got one connection to the X display (see x_erase.c). */
  {
    int saved_frame_number = _plotter->frame_number;

    _g_openpl ();
    _plotter->frame_number = saved_frame_number + 1;
  }

  /* set background color (possibly user-specified) in drawing state */
  bg_color_name_s = (const char *)_get_plot_param ("BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (bg_color_name_s);
  
  /* At this point, we don't clear the drawable(s) by filling them with the
     background color, which is what we would do here for an X Plotter (see
     y_openpl.c).  For an X DrawablePlotter, unlike an X Plotter, initial
     clearing is not appropriate.  However, if we're double buffering, we
     create an off-screen buffer and fill it with the color. */

  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      double_buffer_s = (const char *)_get_plot_param ("USE_DOUBLE_BUFFERING");
      if (strcmp (double_buffer_s, "yes") == 0
	  || strcmp (double_buffer_s, "fast") == 0)
	/* user requested double buffering, so do so `by hand': allocate
	   additional pixmap to serve as off-screen graphics buffer */
	{
	  _plotter->x_double_buffering = DBL_BY_HAND;
	  _plotter->x_drawable3
	    = XCreatePixmap(_plotter->x_dpy, 
			    /* this 2nd arg merely determines the screen*/
			    _plotter->x_drawable1 ? 
			    _plotter->x_drawable1 : _plotter->x_drawable2,
			    (unsigned int)width,
			    (unsigned int)height, 
			    (unsigned int)depth);

	  /* erase buffer by filling it with background color */
	  XFillRectangle (_plotter->x_dpy, _plotter->x_drawable3, 
			  _plotter->drawstate->x_gc_bg,
			  /* upper left corner */
			  0, 0,
			  (unsigned int)width, (unsigned int)height);
	}
    }
  
  return 0;
}

/* This is the XDrawablePlotter-specific version of the
   _maybe_get_new_colormap() method, which is invoked when a Plotter's
   original colormap fills up.  It's a no-op; in XPlotters, it's overridden
   by a version that actually does something. */
void
#ifdef _HAVE_PROTOS
_x_maybe_get_new_colormap (void)
#else
_x_maybe_get_new_colormap ()
#endif
{
  return;
}  

/* This is the XDrawablePlotter-specific version of the
   _maybe_handle_x_events() method, which is invoked after most drawing
   operations.  It's a no-ope; in XPlotters, it's overridden by a version
   that actually does something. */
void
#ifdef _HAVE_PROTOS
_x_maybe_handle_x_events(void)
#else
_x_maybe_handle_x_events()
#endif
{
  return;
}
