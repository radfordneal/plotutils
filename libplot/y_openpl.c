/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. 

   This implementation is for XDrawablePlotters.  It supports one or two
   drawables, which must be associated with the same display and have the
   same dimensions (width, height, depth).  A `drawable' is either a window
   or a pixmap. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_y_openpl (void)
#else
_y_openpl ()
#endif
{
  const char *length_s;
  int screen;			/* screen number */
  Screen *screen_struct;	/* screen structure */

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* If Plotter hasn't been opened before, initialize certain data members
     from parameters; these don't change over the life of a Plotter. */
  if (!_plotter->opened)
    {
      Display *dpy;
      Drawable *drawable_p1, *drawable_p2;

      /* initialize the data members specifying the drawable (a Pixmap or
         Window), and the X display with which it is associated */
      dpy = (Display *)_get_plot_param ("XDRAWABLE_DISPLAY");
      if (dpy == NULL)
	{
	  _plotter->error("XDRAWABLE_DISPLAY parameter is NULL, can't initialize");
	  return -1;
	}
      else
	_plotter->dpy = dpy;
      drawable_p1 = (Drawable *)_get_plot_param ("XDRAWABLE_DRAWABLE1");
      drawable_p2 = (Drawable *)_get_plot_param ("XDRAWABLE_DRAWABLE2");
      if (drawable_p1 == NULL && drawable_p2 == NULL)
	{
	  _plotter->error("XDRAWABLE_DRAWABLE parameters are NULL, can't initialize");
	  return -1;
	}
      else
	{
	  _plotter->drawable1 = *drawable_p1;
	  _plotter->drawable2 = *drawable_p2;
	}

      /* initialize maximum lengths for polylines */
      length_s = (const char *)_get_plot_param ("MAX_LINE_LENGTH");
      {
	int local_length;
	
	if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
	  {
	    _plotter->error("bad MAX_LINE_LENGTH parameter, can't initialize");
	    return -1;
	  }
	else
	  _plotter->max_unfilled_polyline_length = local_length;
      }
      _plotter->hard_polyline_length_limit = 
	XMaxRequestSize(_plotter->dpy) / 2;
  
      /* determine display's default screen, colormap */
      screen = DefaultScreen (_plotter->dpy);
      screen_struct = ScreenOfDisplay (_plotter->dpy, screen);
      _plotter->cmap = DefaultColormapOfScreen (screen_struct);
  
      /* determine dimensions of drawable(s) */
      {
	Window root1, root2;
	int x, y;
	unsigned int width1, height1, width2, height2;
	unsigned int border_width, depth1, depth2;
	unsigned int width, height;
	
	if (_plotter->drawable1)
	  XGetGeometry (_plotter->dpy, _plotter->drawable1,
			&root1, &x, &y, &width1, &height1, &border_width, &depth1);
	if (_plotter->drawable2)
	  XGetGeometry (_plotter->dpy, _plotter->drawable2,
			&root2, &x, &y, &width2, &height2, &border_width, &depth2);

	if (_plotter->drawable1 && _plotter->drawable2)
	  /* sanity check */
	  {
	    if (width1 != width2 || height1 != height2 
		|| depth1 != depth2 || root1 != root2)
	      {
		_plotter->error("can't initialize Plotter, X drawables have unequal parameters");
		return -1;
	      }
	  }

	if (_plotter->drawable1)
	  {
	    width = width1;
	    height = height1;
	  }
	else
	  {
	    width = width2;
	    height = height2;
	  }

	_plotter->imin = 0;
	_plotter->imax = width - 1;
	/* note flipped-y convention for this device: for j, min > max */
	_plotter->jmin = height - 1;
	_plotter->jmax = 0;
      }
      
      /* save background/foreground colors for initial drawing state; store
	 in default drawing state because there's no real drawing state in
	 the Plotter yet */
      _plotter->default_drawstate->x_bgcolor 
	= WhitePixelOfScreen(screen_struct);
      _plotter->default_drawstate->x_fgcolor 
	= BlackPixelOfScreen(screen_struct);
  
    }
  
  /* flag Plotter as open (and having been opened at least once) */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* Create an initial drawing state with default attributes, including the
     just-computed foreground and background colors, and an X GC.  The
     drawing state won't be fully ready for drawing graphics, since it
     won't contain an X font.  To retrieve the initial X font, the user
     will need to invoke space() after openpl(). */
  _plotter->savestate ();
  
  /* erase drawable by filling it with the background color, via
     XFillRectangle (the just-created drawing state, with the mentioned
     attributes, is used for this) */

  /* commented out because for an XDrawablePlotter, unlike an XPlotter,
     this may not be appropriate */
  /* _plotter->erase (); */
  
  return 0;
}
