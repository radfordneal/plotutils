/* This file contains the erase routine, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
erase ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: erase() called when graphics device not open\n");
      return -1;
    }

  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_bgcolor);
  XFillRectangle (_xdata.dpy, _xdata.window, _drawstate->gc,
		  /* upper left corner */
		  0, 0,
		  (unsigned int)_xdata.width, (unsigned int)_xdata.height);
  XFillRectangle (_xdata.dpy, _xdata.pixmap, _drawstate->gc,
		  /* upper left corner */
		  0, 0,
		  (unsigned int)_xdata.width, (unsigned int)_xdata.height);
  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);

  _handle_x_events();
  return 0;
}
