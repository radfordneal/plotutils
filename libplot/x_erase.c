/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_erase (void)
#else
_x_erase ()
#endif
{
  int window_width, window_height;
  XColor rgb;

  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  rgb.red = _plotter->drawstate->bgcolor.red;
  rgb.green = _plotter->drawstate->bgcolor.green;
  rgb.blue = _plotter->drawstate->bgcolor.blue;

  /* allocate color cell */
  if (XAllocColor (_plotter->dpy, _plotter->cmap, &rgb) == 0)
    {
      _plotter->warning ("color request failed, couldn't allocate color cell");
      return -1;
    }

  /* if X server's internal representation of background color is different
     from what we have on file, set background color in X GC */
  if (_plotter->drawstate->x_bgcolor != rgb.pixel)
    {
      _plotter->drawstate->x_bgcolor = rgb.pixel;
      XSetBackground (_plotter->dpy, _plotter->drawstate->gc, rgb.pixel);
    }

  /* compute rectangle size; note flipped-y convention */
  window_width = (_plotter->imax - _plotter->imin) + 1;
  window_height = (_plotter->jmin - _plotter->jmax) + 1;

  /* erase by filling rectangle with background color */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc, 
		  _plotter->drawstate->x_bgcolor);
  if (_plotter->drawable1)
    XFillRectangle (_plotter->dpy, _plotter->drawable1, 
		    _plotter->drawstate->gc,
		    /* upper left corner */
		    0, 0,
		    (unsigned int)window_width, (unsigned int)window_height);
  if (_plotter->drawable2)
    XFillRectangle (_plotter->dpy, _plotter->drawable2, 
		    _plotter->drawstate->gc,
		    /* upper left corner */
		    0, 0,
		    (unsigned int)window_width, (unsigned int)window_height);

  /* restore foreground color in X GC */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc, 
		  _plotter->drawstate->x_fgcolor);

  _handle_x_events();
  return 0;
}
