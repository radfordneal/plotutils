/* This file contains functions that update the bounding box information
   for a page whenever a new object (ellipse, line segment, or Bezier
   segment) is plotted.  Updating takes the line width into account, more
   or less.  The bounding box information is stored in terms of device
   units, in the page's plOutbuf structure. */

/* These functions need to be (non-public) members of the Plotter class,
   since macros such as XD() and YD(), which they use to convert user units
   to device units, access data members of the class.  They are still
   mostly written as if they aren't class members, though. */

#include "sys-defines.h"
#include "extern.h"

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

/* update bounding box due to drawing of ellipse (args are in user coors) */

/* WARNING: This is not completely accurate, due to the nonzero width of
   the pen used to draw the ellipse.  Notoriously, the outer boundary of a
   `wide ellipse' isn't an ellipse at all: it's an eighth-order curve (see
   Foley and van Damm).  Here we approximate it as an ellipse, with
   semimajor and semiminor axes in the user frame increased by one-half of
   the line width.  This approximation is good unless the line width is
   large. */
void
#ifdef _HAVE_PROTOS
_set_ellipse_bbox (R___(Plotter *_plotter) plOutbuf *bufp, double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth)
#else
_set_ellipse_bbox (R___(_plotter) bufp, x, y, rx, ry, costheta, sintheta, linewidth)
     S___(Plotter *_plotter;) 
     plOutbuf *bufp;
     double x, y;
     double rx, ry;
     double costheta, sintheta;
     double linewidth;
#endif
{
  double ux, uy, vx, vy;
  double mixing_angle;
  double semi_axis_1_x, semi_axis_1_y, semi_axis_2_x, semi_axis_2_y;
  double rx_device, ry_device;
  double theta_device, costheta_device, sintheta_device;
  double xdeviation, ydeviation;

  /* take user-frame line width into account (approximately! see above) */
  rx += 0.5 * linewidth;
  ry += 0.5 * linewidth;  

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

  /* angle of inclination of the first semi-axis, in device frame */
  theta_device = - _xatan2 (semi_axis_1_y, semi_axis_1_x);
  costheta_device = cos (theta_device);
  sintheta_device = sin (theta_device);  

  /* maximum displacement in horizontal and vertical directions
     while drawing ellipse, in device frame */
  xdeviation = sqrt (rx_device * rx_device * costheta_device * costheta_device
		     + ry_device * ry_device * sintheta_device * sintheta_device);
  ydeviation = sqrt (rx_device * rx_device * sintheta_device * sintheta_device
		     + ry_device * ry_device * costheta_device * costheta_device);

  /* record these displacements, for bounding box */
  _update_bbox (bufp, XD(x,y) + xdeviation, YD(x,y) + ydeviation);
  _update_bbox (bufp, XD(x,y) + xdeviation, YD(x,y) - ydeviation);
  _update_bbox (bufp, XD(x,y) - xdeviation, YD(x,y) + ydeviation);
  _update_bbox (bufp, XD(x,y) - xdeviation, YD(x,y) - ydeviation);
}

/* update bounding box due to drawing of a line end (args are in user coors) */
void
#ifdef _HAVE_PROTOS
_set_line_end_bbox (R___(Plotter *_plotter) plOutbuf *bufp, double x, double y, double xother, double yother, double linewidth, int capstyle)
#else
_set_line_end_bbox (R___(_plotter) bufp, x, y, xother, yother, linewidth, capstyle)
     S___(Plotter *_plotter;)
     plOutbuf *bufp;
     double x, y, xother, yother, linewidth;
     int capstyle;
#endif
{
  plVector v, vrot;
  double xs, ys;
  double halfwidth = 0.5 * linewidth;

  switch (capstyle)
    {
    case CAP_BUTT:
    default:
      vrot.x = yother - y;
      vrot.y = x - xother;
      _vscale (&vrot, halfwidth);
      xs = x + vrot.x;
      ys = y + vrot.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      xs = x - vrot.x;
      ys = y - vrot.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      break;
    case CAP_PROJECT:
      v.x = xother - x;
      v.y = yother - y;
      _vscale (&v, halfwidth);
      vrot.x = yother - y;
      vrot.y = x - xother;
      _vscale (&vrot, halfwidth);
      xs = x - v.x + vrot.x;
      ys = y - v.y + vrot.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      xs = x - v.x - vrot.x;
      ys = y - v.y - vrot.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      break;
    case CAP_ROUND:
      _set_ellipse_bbox (R___(_plotter) 
			 bufp, x, y, halfwidth, halfwidth, 1.0, 0.0, 0.0);
      break;
    case CAP_TRIANGULAR:
      /* add projecting vertex */
      v.x = xother - x;
      v.y = yother - y;
      _vscale (&v, halfwidth);
      xs = x + v.x;
      ys = y + v.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      /* add other two vertices */
      vrot.x = yother - y;
      vrot.y = x - xother;
      _vscale (&vrot, halfwidth);
      xs = x + vrot.x;
      ys = y + vrot.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      xs = x - vrot.x;
      ys = y - vrot.y;
      _update_bbox (bufp, XD(xs,ys), YD(xs,ys));
      break;
    }
}

/* update bounding box due to drawing of a line join (args are in user coors)*/
void
#ifdef _HAVE_PROTOS
_set_line_join_bbox (R___(Plotter *_plotter) plOutbuf *bufp, double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle, double miterlimit)
#else
_set_line_join_bbox (R___(_plotter) bufp, xleft, yleft, x, y, xright, yright, linewidth, joinstyle, miterlimit)
     S___(Plotter *_plotter;)
     plOutbuf *bufp;
     double xleft, yleft, x, y, xright, yright, linewidth;
     int joinstyle;
     double miterlimit;
#endif
{
  plVector v1, v2, vsum;
  double v1len, v2len;
  double halfwidth;
  double mitrelen;

  switch (joinstyle)
    {
    case JOIN_MITER:
    default:
      v1.x = xleft - x;
      v1.y = yleft - y;
      v2.x = xright - x;
      v2.y = yright - y;
      v1len = VLENGTH(v1);
      v2len = VLENGTH(v2);
      if (v1len == 0.0 || v2len == 0.0)
	_update_bbox (bufp, XD(x,y), YD(x,y));
      else
	{
	  double cosphi;
	  
	  /* The maximum value the cosine of the angle between two joining
	     lines may have, if the join is to be mitered rather than
	     beveled, is 1-2/(M*M), where M is the mitrelimit.  This is
	     because M equals the cosecant of one-half the minimum angle. */
	  cosphi = ((v1.x * v2.x + v1.y * v2.y) / v1len) / v2len;
	  if (miterlimit <= 1.0
	      || (cosphi > (1.0 - 2.0 / (miterlimit * miterlimit))))
	    /* bevel rather than miter */
	    {
	      _set_line_end_bbox (R___(_plotter) 
				  bufp, x, y, xleft, yleft, linewidth, CAP_BUTT);
	      _set_line_end_bbox (R___(_plotter) 
				  bufp,x, y, xright, yright, linewidth, CAP_BUTT);
	    }
	  else
	    {
	      mitrelen = sqrt (1.0 / (2.0 - 2.0 * cosphi)) * linewidth;
	      vsum.x = v1.x + v2.x;
	      vsum.y = v1.y + v2.y;
	      _vscale (&vsum, mitrelen);
	      x -= vsum.x;
	      y -= vsum.y;
	      _update_bbox (bufp, XD(x,y), YD(x,y));
	    }
	}
      break;
    case JOIN_TRIANGULAR:
      /* add a miter vertex, and same vertices as when bevelling */
      vsum.x = v1.x + v2.x;
      vsum.y = v1.y + v2.y;
      _vscale (&vsum, 0.5 * linewidth);
      x -= vsum.x;
      y -= vsum.y;
      _update_bbox (bufp, XD(x,y), YD(x,y));
      /* fall through */
    case JOIN_BEVEL:
      _set_line_end_bbox (R___(_plotter) 
			  bufp, x, y, xleft, yleft, linewidth, CAP_BUTT);
      _set_line_end_bbox (R___(_plotter) 
			  bufp, x, y, xright, yright, linewidth, CAP_BUTT);
      break;
    case JOIN_ROUND:
      halfwidth = 0.5 * linewidth;
      _set_ellipse_bbox (R___(_plotter) 
			 bufp, x, y, halfwidth, halfwidth, 1.0, 0.0, 0.0);
      break;
    }
}

/* Update bounding box due to drawing of a quadratic Bezier segment.  This
   takes into account only extremal x/y values in the interior of the
   segment, i.e. it doesn't take the endpoints into account. */

/* WARNING: Like _set_ellipse_bbox above, this does not properly take line
   width into account.  The boundary of a `thick Bezier' is not a nice
   curve at all. */

#define QUAD_COOR(t,x0,x1,x2) (((x0)-2*(x1)+(x2))*t*t + 2*((x1)-(x2))*t + (x2))

void
#ifdef _HAVE_PROTOS
_set_bezier2_bbox (R___(Plotter *_plotter) plOutbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2)
#else
_set_bezier2_bbox (R___(_plotter) bufp, x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     plOutbuf *bufp;
     double x0, y0, x1, y1, x2, y2;
#endif
{
  double a_x, b_x, t_x;
  double a_y, b_y, t_y;  
  double x, y, xdevice, ydevice;
  double device_halfwidth = 0.5 * _plotter->drawstate->device_line_width;
  
  /* compute coeffs of linear equation at+b=0, for both x and y coors */
  a_x = x0 - 2 * x1 + x2;
  b_x = (x1 - x2);
  a_y = y0 - 2 * y1 + y2;
  b_y = (y1 - y2);
  if (a_x != 0.0)		/* can solve the linear eqn. */
    {
      t_x = -b_x / a_x;
      if (t_x > 0.0 && t_x < 1.0) /* root is in meaningful range */
	{
	  x = QUAD_COOR(t_x, x0, x1, x2);
	  y = QUAD_COOR(t_x, y0, y1, y2);
	  xdevice = XD(x,y);
	  ydevice = YD(x,y);
	  _update_bbox (bufp, xdevice + device_halfwidth, ydevice);
	  _update_bbox (bufp, xdevice - device_halfwidth, ydevice);
	}
    }
  if (a_y != 0.0)		/* can solve the linear eqn. */
    {
      t_y = -b_y / a_y;
      if (t_y > 0.0 && t_y < 1.0) /* root is in meaningful range */
	{
	  x = QUAD_COOR(t_y, x0, x1, x2);
	  y = QUAD_COOR(t_y, y0, y1, y2);
	  xdevice = XD(x,y);
	  ydevice = YD(x,y);
	  _update_bbox (bufp, xdevice, ydevice + device_halfwidth);
	  _update_bbox (bufp, xdevice, ydevice - device_halfwidth);
	}
    }
}

/* Update bounding box due to drawing of a cubic Bezier segment.  This
   takes into account only extremal x/y values in the interior of the
   segment, i.e. it doesn't take the endpoints into account. */

/* WARNING: Like _set_ellipse_bbox above, this does not properly take line
   width into account.  The boundary of a `thick Bezier' is not a nice
   curve at all. */

#define CUBIC_COOR(t,x0,x1,x2,x3) (((x0)-3*(x1)+3*(x2)-(x3))*t*t*t + 3*((x1)-2*(x2)+(x3))*t*t + 3*((x2)-(x3))*t + (x3))

void
#ifdef _HAVE_PROTOS
_set_bezier3_bbox (R___(Plotter *_plotter) plOutbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
_set_bezier3_bbox (R___(_plotter) bufp, x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;) 
     plOutbuf *bufp;
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  double a_x, b_x, c_x, s_x, t_x;
  double a_y, b_y, c_y, s_y, t_y;  
  double x, y, xdevice, ydevice;
  double device_halfwidth = 0.5 * _plotter->drawstate->device_line_width;
  double sqrt_disc;
  
  /* compute coeffs of quad. equation at^2+bt+c=0, for both x and y coors */
  a_x = x0 - 3 * x1 + 3 * x2 - x3;
  b_x = 2 * (x1 - 2 * x2 + x3);
  c_x = x2 - x3;
  a_y = y0 - 3 * y1 + 3 * y2 - y3;
  b_y = 2 * (y1 - 2 * y2 + y3);
  c_y = y2 - y3;
  if (a_x != 0.0)		/* can solve the quadratic */
    {
      sqrt_disc = sqrt (b_x * b_x - 4 * a_x * c_x);
      s_x = (- b_x + sqrt_disc) / (2 * a_x);
      t_x = (- b_x - sqrt_disc) / (2 * a_x);
      if (s_x > 0.0 && s_x < 1.0) /* root is in meaningful range */
	{
	  x = CUBIC_COOR(s_x, x0, x1, x2, x3);
	  y = CUBIC_COOR(s_x, y0, y1, y2, y3);
	  xdevice = XD(x,y);
	  ydevice = YD(x,y);
	  _update_bbox (bufp, xdevice + device_halfwidth, ydevice);
	  _update_bbox (bufp, xdevice - device_halfwidth, ydevice);
	}
      if (t_x > 0.0 && t_x < 1.0) /* root is in meaningful range */
	{
	  x = CUBIC_COOR(t_x, x0, x1, x2, x3);
	  y = CUBIC_COOR(t_x, y0, y1, y2, y3);
	  xdevice = XD(x,y);
	  ydevice = YD(x,y);
	  _update_bbox (bufp, xdevice + device_halfwidth, ydevice);
	  _update_bbox (bufp, xdevice - device_halfwidth, ydevice);
	}
    }
  if (a_y != 0.0)		/* can solve the quadratic */
    {
      sqrt_disc = sqrt (b_y * b_y - 4 * a_y * c_y);
      s_y = (- b_y + sqrt_disc) / (2 * a_y);
      t_y = (- b_y - sqrt_disc) / (2 * a_y);
      if (s_y > 0.0 && s_y < 1.0) /* root is in meaningful range */
	{
	  x = CUBIC_COOR(s_y, x0, x1, x2, x3);
	  y = CUBIC_COOR(s_y, y0, y1, y2, y3);
	  xdevice = XD(x,y);
	  ydevice = YD(x,y);
	  _update_bbox (bufp, xdevice, ydevice + device_halfwidth);
	  _update_bbox (bufp, xdevice, ydevice - device_halfwidth);
	}
      if (t_y > 0.0 && t_y < 1.0) /* root is in meaningful range */
	{
	  x = CUBIC_COOR(t_y, x0, x1, x2, x3);
	  y = CUBIC_COOR(t_y, y0, y1, y2, y3);
	  xdevice = XD(x,y);
	  ydevice = YD(x,y);
	  _update_bbox (bufp, xdevice, ydevice + device_halfwidth);
	  _update_bbox (bufp, xdevice, ydevice - device_halfwidth);
	}
    }
}
