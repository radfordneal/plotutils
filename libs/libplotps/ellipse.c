/* This file contains the ellipse routine, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
{
  double costheta, sintheta;
  double offcenter_rotation_matrix[6];
  double ellipse_transformation_matrix[6];
  double xdeviation, ydeviation;
  double mixing_angle;
  double ux, uy, vx, vy;
  double semi_axis_1_x, semi_axis_1_y;
  double semi_axis_2_x, semi_axis_2_y;  
  double rx_device, ry_device, theta_device;
  double costheta_device, sintheta_device;
  int i;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fellipse() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  /* prologue instruction and idraw directive: start of Elli */
  strcpy (_outbuf.current, "Begin %I Elli\n");
  _update_buffer(&_outbuf);
  
  /* idraw directive: set line bit vector */
  sprintf (_outbuf.current, "%%I b %ld\n", 
	   _ps_line_type_bit_vector[_drawstate->ps_line_type]);
  _update_buffer(&_outbuf);

  sprintf (_outbuf.current, "%d 0 0 [ %s ] 0 SetB\n", 
	   _device_line_width(), 
	   _ps_line_type_setdash[_drawstate->ps_line_type]);
  _update_buffer(&_outbuf);

  /* idraw directive and prologue instruction: set foreground color */
  _evaluate_ps_fgcolor();	/* _evaluated lazily, i.e. when needed */
  sprintf (_outbuf.current, "%%I cfg %s\n%f %f %f SetCFg\n",
	   _idraw_stdcolornames[_drawstate->idraw_fgcolor],
	   _drawstate->ps_fgcolor_red, 
	   _drawstate->ps_fgcolor_green, 
	   _drawstate->ps_fgcolor_blue);
  _update_buffer(&_outbuf);

  /* includes idraw directive: set background color */
  _evaluate_ps_fillcolor();	/* evaluated lazily, i.e. when needed */
  sprintf (_outbuf.current, "%%I cbg %s\n%f %f %f SetCBg\n",
	   _idraw_stdcolornames[_drawstate->idraw_bgcolor],
	   _drawstate->ps_fillcolor_red, 
	   _drawstate->ps_fillcolor_green, 
	   _drawstate->ps_fillcolor_blue);
  _update_buffer(&_outbuf);

  /* includes idraw directive: set fill pattern */
  if (_drawstate->fill_level == 0)	/* transparent */
    sprintf (_outbuf.current, "%%I p\nnone SetP\n");
  else	/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_outbuf.current, "%%I p\n%f SetP\n", 
	     _idraw_stdshadings[_drawstate->idraw_shading]);
  _update_buffer(&_outbuf);

  costheta = cos (M_PI * angle / 180.0);
  sintheta = sin (M_PI * angle / 180.0);

  /* An affine tranformation must be applied to the ellipse produced by the
   Elli routine in the idraw prologue, in order to turn it into the ellipse
   we want.  The Elli routine produces an ellipse with specified semi-axes,
   aligned parallel to the coordinate axes in user space, and centered on 
   the point (x,y).  I.e. it produces, symbolically,

   	[unit circle centered on (0,0)] S T

  where S is a diagonal matrix that scales the unit circle to give the
  specified semi-axis lengths, and T translates (0,0) to (x,y).  This is
  not what we want, since the ellipse is not rotated (it has zero
  inclination angle).  What we want is 
 
   	[unit circle centered on (0,0)] S R T

  where R is a rotation matrix.  This may be rewritten as

   	[unit circle centered on (0,0)] S T  (T^{-1} R T)

  where T^{-1} R T is a so-called offcenter rotation matrix, which rotates
  about the point (x,y).  So the ellipse transformation matrix we'll place
  in the PS code will be (T^{-1} R T) times the matrix that transforms from
  user space to device space. */

  offcenter_rotation_matrix[0] = costheta; /* 1st 4 elements are those of R */
  offcenter_rotation_matrix[1] = sintheta;
  offcenter_rotation_matrix[2] = - sintheta;
  offcenter_rotation_matrix[3] = costheta;
  offcenter_rotation_matrix[4] = x * (1.0 - costheta) + y * sintheta;
  offcenter_rotation_matrix[5] = y * (1.0 - costheta) - x * sintheta;
  
  _matrix_product (offcenter_rotation_matrix, _drawstate->transform.m,
		   ellipse_transformation_matrix);
  
  /* includes idraw directive: transformation matrix (all 6 elements) */
  sprintf (_outbuf.current, "%%I t\n[");
  _update_buffer(&_outbuf);
  for (i = 0; i < 6; i++)
    {
      if ((i==0) || (i==1) || (i==2) || (i==3))
	sprintf (_outbuf.current, "%f ", ellipse_transformation_matrix[i] / GRANULARITY);
      else
	sprintf (_outbuf.current, "%f ", ellipse_transformation_matrix[i]);
      _update_buffer(&_outbuf);
    }
  sprintf (_outbuf.current, "] concat\n");
  _update_buffer(&_outbuf);

  /* includes idraw directive: draw Elli, and end Elli */
  sprintf (_outbuf.current, "%%I\n%d %d %d %d Elli\nEnd\n\n", 
	   IROUND(GRANULARITY * x), IROUND(GRANULARITY * y), 
	   IROUND(GRANULARITY * rx), IROUND(GRANULARITY * ry));
  _update_buffer(&_outbuf);

  /* Now start computing bounding box */

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
  
  (_drawstate->pos).x = x;	/* move to center of ellipse */
  (_drawstate->pos).y = y;

  return 0;
}

int
ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
{
  return fellipse ((double)x, (double)y, 
		   (double)rx, (double)ry, (double)angle);
}
