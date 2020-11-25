/* This file contains the circle routine, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r.

   This file also contains the ellipse routine, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis).
   In libplotfig, these both call _draw_ellipse_internal(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* subtypes of ELLIPSE object */

#define SUBTYPE_ELLIPSE 1	/* subtype: ellipse defined by radii */
#define SUBTYPE_CIRCLE  3	/* subtype: circle defined by radius */

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static int _draw_ellipse_internal P__((double x, double y, double rx, double ry, double angle, int subtype));
#undef P__

static int
_draw_ellipse_internal (x, y, rx, ry, angle, subtype)
     double x, y, rx, ry, angle;
     int subtype;
{
  double theta, mixing_angle;
  double ux, uy, vx, vy;
  double semi_axis_1_x, semi_axis_1_y;
  double semi_axis_2_x, semi_axis_2_y;  
  double rx_device, ry_device, theta_device;
  double costheta, sintheta;
  char *format;

  /* inclination angle (radians), in user frame */
  theta = M_PI * angle / 180.0;
  costheta = cos (theta);
  sintheta = sin (theta);

  /* perform affine user->device coor transformation; (ux,uy) and (vx,vy)
     are forward images of the semiaxes, i.e. they are conjugate radial
     vectors in the device frame */

  ux = XDV(rx * costheta, rx * sintheta);
  uy = YDV(rx * costheta, rx * sintheta);

  vx = XDV(-ry * sintheta, ry * costheta);
  vy = YDV(-ry * sintheta, ry * costheta);

  /* angle by which the conjugate radial vectors should be mixed, in order
     to yield vectors along the major and minor axes in the device frame */
  mixing_angle = 0.5 * _xatan2 (2.0 * (ux * vx + uy * vy),
				ux * ux + uy * uy - vx * vx + vy * vy);
  
  /* semi-axis vectors in device coordinates */
  semi_axis_1_x = ux * cos(mixing_angle) + vx * sin(mixing_angle);
  semi_axis_1_y = uy * cos(mixing_angle) + vy * sin(mixing_angle);  
  semi_axis_2_x = ux * cos(mixing_angle + M_PI_2) 
    + vx * sin(mixing_angle + M_PI_2);
  semi_axis_2_y = uy * cos(mixing_angle + M_PI_2) 
    + vy * sin(mixing_angle + M_PI_2);  

  /* semi-axis lengths in device coordinates */
  rx_device = sqrt (semi_axis_1_x * semi_axis_1_x
		    + semi_axis_1_y * semi_axis_1_y);
  ry_device = sqrt (semi_axis_2_x * semi_axis_2_x
		    + semi_axis_2_y * semi_axis_2_y);

  /* angle of inclination of the first semi-axis, in device frame
     (note flipped-y convention) */
  theta_device = - _xatan2 (semi_axis_1_y, semi_axis_1_x);
  if (theta_device == 0.0)
    theta_device = 0.0;		/* remove sign bit if any */

  if (subtype == SUBTYPE_CIRCLE && 
      IROUND (rx_device) != IROUND (ry_device))
    subtype = SUBTYPE_ELLIPSE;

  /* evaluate fig colors lazily, i.e. only when needed */
  _evaluate_fig_fgcolor();
  _evaluate_fig_fillcolor();

  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_drawstate->fig_last_priority > ELLIPSE_PRIORITY)
    if (_drawstate->fig_drawing_depth > 0)
      (_drawstate->fig_drawing_depth)--;
  _drawstate->fig_last_priority = ELLIPSE_PRIORITY;

  if (subtype == SUBTYPE_CIRCLE)
    format = "#ELLIPSE [CIRCLE]\n%d %d %d %d %d %d %d %d %d %.3f %d %.3f %d %d %d %d %d %d %d %d\n";
  else
    format = "#ELLIPSE\n%d %d %d %d %d %d %d %d %d %.3f %d %.3f %d %d %d %d %d %d %d %d\n";

  sprintf(_outbuf.current,
	  format,
	  1,			/* ellipse object */
	  subtype,		/* subtype, see above */
	  _drawstate->fig_line_style, /* style */
	  _device_line_width(), /* thickness, in Fig display units */
	  _drawstate->fig_fgcolor,	/* pen color */
	  _drawstate->fig_fillcolor, /* fill color */
	  _drawstate->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _drawstate->fig_fill_level, /* area fill */
	  _drawstate->fig_dash_length, /* style val, in Fig display units */
	  1,			/* direction, always 1 */
	  theta_device,		/* inclination angle, in radians (float) */
	  IROUND(XD(x,y)),	/* center_x (not float, unlike arc) */
	  IROUND(YD(x,y)),	/* center_y (not float, unlike arc) */
	  IROUND(rx_device),	/* radius_x */
	  IROUND(ry_device),	/* radius_y */
	  IROUND(XD(x,y)),	/* start_x, 1st point entered */
	  IROUND(YD(x,y)),	/* start_y, 1st point entered */
	  IROUND(XD(x,y)	/* end_x, last point entered */
		 + semi_axis_1_x + semi_axis_2_x),
	  IROUND(YD(x,y)	/* end_y, last point entered */
		 + semi_axis_1_y + semi_axis_2_y) 
	  );			
  _update_buffer(&_outbuf);
  
  (_drawstate->pos).x = x;	/* move to center of ellipse or circle */
  (_drawstate->pos).y = y;

  return 0;
}

int
fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipse() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  return _draw_ellipse_internal (x, y, rx, ry, angle, SUBTYPE_ELLIPSE);
}

int
ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
{
  return fellipse ((double)x, (double)y, (double)rx, (double)ry, (double)angle);
}

int
fcircle (x, y, r)
     double x, y, r;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcircle() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  return _draw_ellipse_internal (x, y, r, r, 0.0, SUBTYPE_CIRCLE);
}

int
circle (x, y, r)
     int x, y, r;
{
  return ellipse (x, y, r, r, 0);
}
