/* This file contains functions that update the bounding box information
   whenever a new object (point, ellipse or polyline) is plotted.  Updating
   takes the line width with which the object is drawn into account.

   Internal accounting is in terms of device units, i.e. printer's
   points. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* MITRE_COSINE_LIMIT is the maximum value the cosine of the angle between
   two joining lines may have, if the join is to be mitered rather than
   beveled.  In general it equals 1-2/(M*M), where M is the Postscript
   mitrelimit.  By default, M=10.0. */
#define MITRE_COSINE_LIMIT 0.98

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

/* reset bounding box, called by openpl(), erase(), closepl() */
void 
#ifdef _HAVE_PROTOS
_reset_range (void)
#else
_reset_range ()
#endif
{
  _plotter->xrange_max = -(MAXDOUBLE);
  _plotter->xrange_min = MAXDOUBLE;
  _plotter->yrange_max = -(MAXDOUBLE);
  _plotter->yrange_min = MAXDOUBLE;
}

/* query bounding box (returned values are in device coordinates) */
void 
#ifdef _HAVE_PROTOS
_get_range (double *xmin, double *xmax, double *ymin, double *ymax)
#else
_get_range (xmin, xmax, ymin, ymax)
     double *xmin, *xmax, *ymin, *ymax;
#endif
{
  if ((_plotter->xrange_max < _plotter->xrange_min) 
      || (_plotter->yrange_max < _plotter->yrange_min)) /* no objects */
    {
      *xmax = 0.0;
      *xmin = 0.0;
      *ymax = 0.0;
      *ymin = 0.0;
    }
  else
    {
      *xmax = _plotter->xrange_max;
      *xmin = _plotter->xrange_min;
      *ymax = _plotter->yrange_max;
      *ymin = _plotter->yrange_min;
    }
}

/* update bounding box due to drawing of a point (args are in device
   coordinates) */
void 
#ifdef _HAVE_PROTOS
_set_range (double x, double y)
#else
_set_range (x, y)
     double x, y;
#endif
{
  if (x > _plotter->xrange_max) _plotter->xrange_max = x;
  if (x < _plotter->xrange_min) _plotter->xrange_min = x;
  if (y > _plotter->yrange_max) _plotter->yrange_max = y;
  if (y < _plotter->yrange_min) _plotter->yrange_min = y;
}

/* update bounding box due to drawing of ellipse (args are in user coors) */
void
#ifdef _HAVE_PROTOS
_set_ellipse_bbox (double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth)
#else
_set_ellipse_bbox (x, y, rx, ry, costheta, sintheta, linewidth)
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
  _set_range (XD(x,y) + xdeviation, YD(x,y) + ydeviation);
  _set_range (XD(x,y) + xdeviation, YD(x,y) - ydeviation);
  _set_range (XD(x,y) - xdeviation, YD(x,y) + ydeviation);
  _set_range (XD(x,y) - xdeviation, YD(x,y) - ydeviation);
}

/* update bounding box due to drawing of a line end (args are in user coors) */
void
#ifdef _HAVE_PROTOS
_set_line_end_bbox (double x, double y, double xother, double yother, double linewidth, int capstyle)
#else
_set_line_end_bbox (x, y, xother, yother, linewidth, capstyle)
     double x, y, xother, yother, linewidth;
     int capstyle;
#endif
{
  Vector v, vrot;
  double xs, ys;
  double halfwidth = 0.5 * linewidth;

  switch (capstyle)
    {
    case PS_CAP_BUTT:
    default:
      vrot.x = yother - y;
      vrot.y = x - xother;
      _vscale (&vrot, halfwidth);
      xs = x + vrot.x;
      ys = y + vrot.y;
      _set_range (XD(xs,ys), YD(xs,ys));
      xs = x - vrot.x;
      ys = y - vrot.y;
      _set_range (XD(xs,ys), YD(xs,ys));
      break;
    case PS_CAP_PROJECT:
      v.x = xother - x;
      v.y = yother - y;
      _vscale (&v, halfwidth);
      vrot.x = yother - y;
      vrot.y = x - xother;
      _vscale (&vrot, halfwidth);
      xs = x - v.x + vrot.x;
      ys = y - v.y + vrot.y;
      _set_range (XD(xs,ys), YD(xs,ys));
      xs = x - v.x - vrot.x;
      ys = y - v.y - vrot.y;
      _set_range (XD(xs,ys), YD(xs,ys));
      break;
    case PS_CAP_ROUND:
      _set_ellipse_bbox (x, y, halfwidth, halfwidth, 1.0, 0.0, 0.0);
      break;
    }
}

/* update bounding box due to drawing of a line join (args are in user coors)*/
void
#ifdef _HAVE_PROTOS
_set_line_join_bbox (double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle)
#else
_set_line_join_bbox (xleft, yleft, x, y, xright, yright, linewidth, joinstyle)
     double xleft, yleft, x, y, xright, yright, linewidth;
     int joinstyle;
#endif
{
  double halfwidth = 0.5 * linewidth;
  Vector v1, v2;
  double v1len, v2len;

  switch (joinstyle)
    {
    case PS_JOIN_MITER:
    default:
      v1.x = xleft - x;
      v1.y = yleft - y;
      v2.x = xright - x;
      v2.y = yright - y;
      v1len = VLENGTH(v1);
      v2len = VLENGTH(v2);
      if (v1len == 0.0 || v2len == 0.0)
	_set_range (XD(x,y), YD(x,y));
      else
	{
	  double cosphi;
	  
	  cosphi = fabs (((v1.x * v2.x + v1.y * v2.y) / v1len) / v2len);
	  if (cosphi > MITRE_COSINE_LIMIT)
	    /* bevel rather than miter */
	    {
	      _set_line_end_bbox (x, y, xleft, yleft, linewidth, PS_CAP_BUTT);
	      _set_line_end_bbox (x, y, xright, yright, linewidth, PS_CAP_BUTT);
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
	      _set_range (XD(x,y), YD(x,y));
	    }
	}
      break;
    case PS_JOIN_BEVEL:
      _set_line_end_bbox (x, y, xleft, yleft, linewidth, PS_CAP_BUTT);
      _set_line_end_bbox (x, y, xright, yright, linewidth, PS_CAP_BUTT);
      break;
    case PS_JOIN_ROUND:
      _set_ellipse_bbox (x, y, halfwidth, halfwidth, 1.0, 0.0, 0.0);
      break;
    }
}
