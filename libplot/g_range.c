/* This file contains functions that update the bounding box information
   for a page whenever a new object (ellipse or line segment) is plotted.
   Updating takes the line width into account.  The bounding box
   information is stored in terms of device units, in the page's Outbuf
   structure. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* MITRE_COSINE_LIMIT is the maximum value the cosine of the angle between
   two joining lines may have, if the join is to be mitered rather than
   beveled.  In general it equals 1-2/(M*M), where M is the mitrelimit.  We
   assume M=10.0, which is correct for Postscript. */
#define MITRE_COSINE_LIMIT 0.98

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

/* update bounding box due to drawing of ellipse (args are in user coors) */
void
#ifdef _HAVE_PROTOS
_set_ellipse_bbox (Outbuf *bufp, double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth)
#else
_set_ellipse_bbox (bufp, x, y, rx, ry, costheta, sintheta, linewidth)
     Outbuf *bufp;
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

  /* take user-frame line width into account */
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
  _set_range (bufp, XD(x,y) + xdeviation, YD(x,y) + ydeviation);
  _set_range (bufp, XD(x,y) + xdeviation, YD(x,y) - ydeviation);
  _set_range (bufp, XD(x,y) - xdeviation, YD(x,y) + ydeviation);
  _set_range (bufp, XD(x,y) - xdeviation, YD(x,y) - ydeviation);
}

/* update bounding box due to drawing of a line end (args are in user coors) */
void
#ifdef _HAVE_PROTOS
_set_line_end_bbox (Outbuf *bufp, double x, double y, double xother, double yother, double linewidth, int capstyle)
#else
_set_line_end_bbox (bufp, x, y, xother, yother, linewidth, capstyle)
     Outbuf *bufp;
     double x, y, xother, yother, linewidth;
     int capstyle;
#endif
{
  Vector v, vrot;
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
      _set_range (bufp, XD(xs,ys), YD(xs,ys));
      xs = x - vrot.x;
      ys = y - vrot.y;
      _set_range (bufp, XD(xs,ys), YD(xs,ys));
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
      _set_range (bufp, XD(xs,ys), YD(xs,ys));
      xs = x - v.x - vrot.x;
      ys = y - v.y - vrot.y;
      _set_range (bufp, XD(xs,ys), YD(xs,ys));
      break;
    case CAP_ROUND:
      _set_ellipse_bbox (bufp, x, y, halfwidth, halfwidth, 1.0, 0.0, 0.0);
      break;
    }
}

/* update bounding box due to drawing of a line join (args are in user coors)*/
void
#ifdef _HAVE_PROTOS
_set_line_join_bbox (Outbuf *bufp, double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle)
#else
_set_line_join_bbox (bufp, xleft, yleft, x, y, xright, yright, linewidth, joinstyle)
     Outbuf *bufp;
     double xleft, yleft, x, y, xright, yright, linewidth;
     int joinstyle;
#endif
{
  double halfwidth = 0.5 * linewidth;
  Vector v1, v2;
  double v1len, v2len;

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
	_set_range (bufp, XD(x,y), YD(x,y));
      else
	{
	  double cosphi;
	  
	  cosphi = fabs (((v1.x * v2.x + v1.y * v2.y) / v1len) / v2len);
	  if (cosphi > MITRE_COSINE_LIMIT)
	    /* bevel rather than miter */
	    {
	      _set_line_end_bbox (bufp, x, y, xleft, yleft, linewidth, CAP_BUTT);
	      _set_line_end_bbox (bufp,x, y, xright, yright, linewidth, CAP_BUTT);
	    }
	  else
	    {
	      double mitrelen;
	      Vector vsum;
	      
	      mitrelen = sqrt (1.0 / (2.0 - 2.0 * cosphi)) * linewidth;
	      vsum.x = v1.x + v2.x;
	      vsum.y = v1.y + v2.y;
	      _vscale (&vsum, mitrelen);
	      x -= vsum.x;
	      y -= vsum.y;
	      _set_range (bufp, XD(x,y), YD(x,y));
	    }
	}
      break;
    case JOIN_BEVEL:
      _set_line_end_bbox (bufp, x, y, xleft, yleft, linewidth, CAP_BUTT);
      _set_line_end_bbox (bufp, x, y, xright, yright, linewidth, CAP_BUTT);
      break;
    case JOIN_ROUND:
      _set_ellipse_bbox (bufp, x, y, halfwidth, halfwidth, 1.0, 0.0, 0.0);
      break;
    }
}
