/* This file contains the ellipse routine, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* For libplotX, we first check to see if the angle of inclination is zero,
   and the map from the user frame to the device frame preserves axes.  If
   these are both true, we can use native X rendering to draw the ellipse.
   If they are not, we draw an inscribed polyline, by calling
   _draw_ellipse().

   For the duration of this polyline, we set _suppress_polyline_flushout to
   TRUE, so that the X server will be sure to see the polyline as a closed
   polygon.  (We normally permit polylines [unfilled ones anyway] to be
   flushed out even as they are being added to, if they get too long.
   We don't want this to happen; we want always to send the ellipse to
   the server as a single X request.)
*/

int
fellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
{
  int ninetymult = IROUND(angle / 90.0);

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipse() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure X problems */
  ry = (ry < 0.0 ? -ry : ry);  

 /* if angle is multiple of 90 degrees, modify to permit use of native X
    arc rendering */
  if (angle == (double) (90 * ninetymult))
    {
      angle = 0.0;
      if (ninetymult % 2)
	{
	  double t;
	  
	  t = rx;
	  rx = ry;
	  ry = t;
	}
    }

  if ((_drawstate->transform.axes_preserved) 
      && (angle == 0.0))	/* CASE 1: use native X arc rendering */
    {
      int xorigin, yorigin;
      unsigned int xdiam, ydiam;

      xorigin = IROUND(XD(xc - rx, yc + ry));
      yorigin = IROUND(YD(xc - rx, yc + ry));
      xdiam = (unsigned int)IROUND(XDV(2 * rx, 0.0));
      /* note flipped-y convention */
      ydiam = (unsigned int)IROUND(YDV(0.0, -2 * ry));  
      
      /* don't use zero dimensions if user specified nonzero */
      if (xdiam == 0 && rx > 0.0)
	xdiam = 1;
      if (ydiam == 0 && ry > 0.0)
	ydiam = 1;

      if (_drawstate->fill_level)	/* not transparent */
	{
	  _evaluate_x_fillcolor();
	  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fillcolor);
	  XFillArc(_xdata.dpy, _xdata.window, _drawstate->gc, 
		   xorigin, yorigin, xdiam, ydiam, 0, 64 * 360);
	  XFillArc(_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		   xorigin, yorigin, xdiam, ydiam, 0, 64 * 360);
	}
      
      _evaluate_x_fgcolor();
      XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
      XDrawArc(_xdata.dpy, _xdata.window, _drawstate->gc, 
		   xorigin, yorigin, xdiam, ydiam, 0, 64 * 360);
      XDrawArc(_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		   xorigin, yorigin, xdiam, ydiam, 0, 64 * 360);
    }

  else		/* CASE 2: inscribe a polyline in the ellipse */
    {
      _suppress_polyline_flushout = TRUE;

      _draw_ellipse (xc, yc, rx, ry, angle);

      _suppress_polyline_flushout = FALSE; /* restore */

      endpoly();		/* flush out polyline */
    }
  
  (_drawstate->pos).x = xc;	/* move to center (a libplot convention) */
  (_drawstate->pos).y = yc;

  _handle_x_events();

  return 0;
}

int
ellipse (x, y, xradius, yradius, angle)
     int x, y, xradius, yradius, angle;
{
  return fellipse ((double)x, (double)y, 
		   (double)xradius, (double)yradius, (double)angle);
}
