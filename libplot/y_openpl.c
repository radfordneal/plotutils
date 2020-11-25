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

  /* determine dimensions of drawable(s) */
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
	  _plotter->error("can't open Plotter, X drawables have unequal parameters");
	  return false;
	}
    }
  
  if (_plotter->drawable1)
    {
      width = width1;
      height = height1;
      depth = depth1;
    }
  else if (_plotter->drawable2)
    {
      width = width2;
      height = height2;
      depth = depth1;
    }
  else
  /* if both are NULL, we won't be creating a pixmap or drawing, so these
     are irrelevant */
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
  
  if (_plotter->drawable1 || _plotter->drawable2)
    {
      double_buffer_s = (const char *)_get_plot_param (_plotter, "USE_DOUBLE_BUFFERING");
      if (strcmp (double_buffer_s, "yes") == 0
	  || strcmp (double_buffer_s, "fast") == 0)
	/* user requested double buffering, so do so `by hand': allocate
	   additional pixmap to serve as off-screen graphics buffer */
	{
	  _plotter->double_buffering = DBL_BY_HAND;
	  _plotter->drawable3
	    = XCreatePixmap(_plotter->dpy, 
			    /* this 2nd arg merely determines the screen*/
			    _plotter->drawable1 ? 
			    _plotter->drawable1 : _plotter->drawable2,
			    (unsigned int)width,
			    (unsigned int)height, 
			    (unsigned int)depth);
	  /* erase buffer by filling it with background color */
	  XFillRectangle (_plotter->dpy, _plotter->drawable3, 
			  _plotter->drawstate->gc_bg,
			  /* upper left corner */
			  0, 0,
			  (unsigned int)width, (unsigned int)height);
	}
    }
  
  /* flag Plotter as open (and having been opened at least once) */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

  /* Note: we do _not_ reset _plotter->frame_number to zero here, unlike
     what we do for an X Plotter.  Frames for an X Drawable Plotter are
     (currently) numbered consecutively throughout the lifetime of the
     Plotter. */

  /* Create an initial drawing state with default attributes, including an
     X GC.  The drawing state won't be ready for drawing graphics, since it
     won't contain an X font or a meaningful line width.  To retrieve an X
     font and set the line width, the user will need to invoke space()
     after openpl(). */
  _plotter->savestate ();
  
  /* if there's a user-specified background color, set it in drawing state */
  bg_color_name_s = (const char *)_get_plot_param (_plotter, "BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (bg_color_name_s);
  
  /* We don't clear the drawable(s) by filling them with the background
     color.  For an X DrawablePlotter, unlike an X Plotter, initial
     clearing is not appropriate. */

  return 0;
}
