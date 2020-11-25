/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r.

   This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis).
   Both routines produce output in fig format, and both call
   _f_draw_ellipse_internal(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* subtypes of ELLIPSE object */

#define SUBTYPE_ELLIPSE 1	/* subtype: ellipse defined by radii */
#define SUBTYPE_CIRCLE  3	/* subtype: circle defined by radius */

static int _f_draw_ellipse_internal __P((double x, double y, double rx, double ry, double angle, int subtype));

static int
#ifdef _HAVE_PROTOS
_f_draw_ellipse_internal (double x, double y, double rx, double ry, double angle, int subtype)
#else
_f_draw_ellipse_internal (x, y, rx, ry, angle, subtype)
     double x, y, rx, ry, angle;
     int subtype;
#endif
{
  double theta, mixing_angle;
  double ux, uy, vx, vy;
  double semi_axis_1_x, semi_axis_1_y;
  double semi_axis_2_x, semi_axis_2_y;  
  double rx_device, ry_device, theta_device;
  double costheta, sintheta;
  const char *format;

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
  _plotter->set_pen_color();
  _plotter->set_fill_color();
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_plotter->fig_last_priority >= ELLIPSE_PRIORITY)
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;
  _plotter->fig_last_priority = ELLIPSE_PRIORITY;

  if (subtype == SUBTYPE_CIRCLE)
    format = "#ELLIPSE [CIRCLE]\n%d %d %d %d %d %d %d %d %d %.3f %d %.3f %d %d %d %d %d %d %d %d\n";
  else
    format = "#ELLIPSE\n%d %d %d %d %d %d %d %d %d %.3f %d %.3f %d %d %d %d %d %d %d %d\n";

  sprintf(_plotter->page->point,
	  format,
	  1,			/* ellipse object */
	  subtype,		/* subtype, see above */
	  _fig_line_style[_plotter->drawstate->line_type], /* style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor,	/* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
		  /* style val, in Fig display units (float) */
	  _fig_dash_length[_plotter->drawstate->line_type], 
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
  _update_buffer(_plotter->page);
  
  _plotter->drawstate->pos.x = x; /* move to center of ellipse or circle */
  _plotter->drawstate->pos.y = y;

  return 0;
}

int
#ifdef _HAVE_PROTOS
_f_fellipse (double x, double y, double rx, double ry, double angle)
#else
_f_fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing */
    return 0;

  return _f_draw_ellipse_internal (x, y, rx, ry, angle, SUBTYPE_ELLIPSE);
}

int
#ifdef _HAVE_PROTOS
_f_fcircle (double x, double y, double r)
#else
_f_fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcircle: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing */
    return 0;

  return _f_draw_ellipse_internal (x, y, r, r, 0.0, SUBTYPE_CIRCLE);
}
