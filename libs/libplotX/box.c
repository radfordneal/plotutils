/* This file contains the box routine, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  double xmin, xmax;
  double ymin, ymax;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fbox() called when graphics device not open\n");
      return -1;
    }

  /* if user coors -> device coors transformation does not preserve axis
     directions, draw closed polyline rather than using the native X
     rectangle-drawing facility (which aligns boxes with the axes) */
  if (!_drawstate->transform.axes_preserved)
    {
      fmove (x0, y0);
      fcont (x0, y1);
      fcont (x1, y1);
      fcont (x1, y0);
      fcont (x0, y0);
      fmove (x1, y1);
      return 0;
    }

  endpoly ();			/* flush polyline if any */

  xmin = DMIN (x0, x1);
  ymin = DMIN (y0, y1);
  xmax = DMAX (x0, x1);
  ymax = DMAX (y0, y1);

  /* Semantics of XDrawRectangle() and XFillRectangle() are as follows.  To
     draw a 10x10 rectangle, the boundary of the rectangular region
     [0,9]x[0,9], XDrawRectangle() should be called with arguments 0,0,9,9.
     But to paint each pixel in [0,9]x[0,9], XFillRectangle() should be
     called with args 0,0,10,10.

     We don't bother incrementing the args of XFillRectangle, because we
     follow our XFillRectangle() by an XDrawRectangle(). */

  if (_drawstate->fill_level)	/* not transparent */
    {
      _evaluate_x_fillcolor();
      XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fillcolor);
      XFillRectangle (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		      IROUND(XD (xmin, ymax)), 
		      IROUND(YD (xmin, ymax)),
		      (unsigned int)(IROUND(XD (xmax, ymax))
				     - IROUND(XD (xmin, ymax))),
		      /* note flipped-y convention */
		      (unsigned int)(IROUND(YD (xmin, ymin))
				     - IROUND(YD (xmin, ymax))));
      XFillRectangle (_xdata.dpy, _xdata.window, _drawstate->gc, 
		      IROUND(XD (xmin, ymax)), 
		      IROUND(YD (xmin, ymax)),
		      (unsigned int)(IROUND(XD (xmax, ymax))
				     - IROUND(XD (xmin, ymax))),
		      /* note flipped-y convention */
		      (unsigned int)(IROUND(YD (xmin, ymin))
				     - IROUND(YD (xmin, ymax))));
    }

  _evaluate_x_fgcolor();
  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
  XDrawRectangle (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		  IROUND(XD (xmin, ymax)), 
		  IROUND(YD (xmin, ymax)),
		  (unsigned int)(IROUND(XD (xmax, ymax))
				 - IROUND(XD (xmin, ymax))),
		  /* note flipped-y convention */
		  (unsigned int)(IROUND(YD (xmin, ymin))
				 - IROUND(YD (xmin, ymax))));
  XDrawRectangle (_xdata.dpy, _xdata.window, _drawstate->gc, 
		  IROUND(XD (xmin, ymax)), 
		  IROUND(YD (xmin, ymax)),
		  (unsigned int)(IROUND(XD (xmax, ymax))
				 - IROUND(XD (xmin, ymax))),
		  /* note flipped-y convention */
		  (unsigned int)(IROUND(YD (xmin, ymin))
				 - IROUND(YD (xmin, ymax))));

  (_drawstate->pos).x = x1;	/* update our notion of position */
  (_drawstate->pos).y = y1;

  _handle_x_events();

  return 0;
}

int
box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  return fbox ((double)x0, (double)y0, (double)x1, (double)y1);
}
