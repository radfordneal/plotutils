/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

/* This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis).
   Both methods produce output in PS format, and both call
   _p_draw_ellipse_internal(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* forward references */
void _p_fellipse_internal __P((double x, double y, double rx, double ry, double angle, bool circlep));

int
#ifdef _HAVE_PROTOS
_p_fcircle (double x, double y, double radius)
#else
_p_fcircle (x, y, radius)
     double x, y, radius;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcircle: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* final arg flags this for idraw as a circle, not an ellipse */
  _p_fellipse_internal (x, y, radius, radius, 0.0, true);

  return 0;
}

int
#ifdef _HAVE_PROTOS
_p_fellipse (double x, double y, double rx, double ry, double angle)
#else
_p_fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* final arg flags this for idraw as an ellipse, not a circle */
  _p_fellipse_internal (x, y, rx, ry, angle, false);

  return 0;
}

void
#ifdef _HAVE_PROTOS
_p_fellipse_internal (double x, double y, double rx, double ry, double angle, bool circlep)
#else
_p_fellipse_internal (x, y, rx, ry, angle, circlep)
     double x, y, rx, ry, angle;
     bool circlep;		/* drawn as a circle in user frame? */
#endif
{  
  double costheta, sintheta;
  double offcenter_rotation_matrix[6];
  double ellipse_transformation_matrix[6];
  double invnorm = 0.0, granularity = 1.0;
  bool singular_map;
  int i;

  /* compute reciprocal norm of user->device affine transformation; need
     this because transformation matrices we'll emit must be normalized */
  {
    double det, norm;
    
    det = _plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3]
      - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2];
    norm = sqrt(fabs(det));
    /* granularity = scaleup factor for user coordinates, so that when
       they're emitted as integers, resolution loss won't be excessive.
       CTM entries will be scaled down by this factor. */
    granularity = norm / (PS_MIN_RESOLUTION);
    if (norm != 0.0)
      {
	/* invnorm is `norm' of device->user coordinate transformation */
	invnorm = 1.0 / norm;
	singular_map = false;
      }
    else
      singular_map = true;
  }

  /* prologue instruction and idraw directive: start of Elli or Circ */
  if (circlep)
    strcpy (_plotter->outbuf.current, "Begin %I Circ\n");
  else
    strcpy (_plotter->outbuf.current, "Begin %I Elli\n");
  _update_buffer(&_plotter->outbuf);
  
  /* redefine `originalCTM' matrix (relevant to line width only) */
  if (singular_map != true)
    {
      int integer_linewidth = _plotter->drawstate->quantized_device_line_width;
      double double_linewidth = _plotter->drawstate->device_line_width;
      double linewidth_adjust;

      /* adjustment needed, due to our specifying line widths as integers */
      if (integer_linewidth != 0)
	linewidth_adjust = double_linewidth / integer_linewidth;
      else
	linewidth_adjust = 1.0;

      sprintf (_plotter->outbuf.current, "[");
      _update_buffer (&_plotter->outbuf);

      for (i = 0; i < 4; i++)
	{
	  sprintf (_plotter->outbuf.current, "%.7g ", 
		   linewidth_adjust * invnorm * _plotter->drawstate->transform.m[i]);
	  _update_buffer (&_plotter->outbuf);
	}
      _update_buffer (&_plotter->outbuf);
      sprintf (_plotter->outbuf.current, 
	       "0 0 ] trueoriginalCTM originalCTM\n concatmatrix pop\n");
      _update_buffer (&_plotter->outbuf);
    }

  /* specify cap style and join style */
  sprintf (_plotter->outbuf.current, "%d setlinecap %d setlinejoin\n",
	   _ps_cap_style[_plotter->drawstate->cap_type], 
	   _ps_join_style[_plotter->drawstate->join_type]);
  _update_buffer (&_plotter->outbuf);

  /* idraw directive: set line bit vector */
  sprintf (_plotter->outbuf.current, "%%I b %ld\n", 
	   _ps_line_type_bit_vector[_plotter->drawstate->line_type]);
  _update_buffer(&_plotter->outbuf);

  /* PS instruction: SetB (i.e. setbrush), with args
     LineWidth, LeftArrow, RightArrow, DashArray, DashOffset. */
  /* Note LineWidth must be an integer for idraw compatibility. */
  sprintf (_plotter->outbuf.current, "%d 0 0 [ %s ] 0 SetB\n", 
	   _plotter->drawstate->quantized_device_line_width,
	   _ps_line_type_setdash[_plotter->drawstate->line_type]);
  _update_buffer(&_plotter->outbuf);

  /* idraw directive and prologue instruction: set foreground color */
  _plotter->set_pen_color();	/* invoked lazily, i.e. when needed */
  sprintf (_plotter->outbuf.current, "%%I cfg %s\n%g %g %g SetCFg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red, 
	   _plotter->drawstate->ps_fgcolor_green, 
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer(&_plotter->outbuf);

  /* includes idraw directive: set background color */
  _plotter->set_fill_color();	/* invoked lazily, i.e. when needed */
  sprintf (_plotter->outbuf.current, "%%I cbg %s\n%g %g %g SetCBg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_bgcolor],
	   _plotter->drawstate->ps_fillcolor_red, 
	   _plotter->drawstate->ps_fillcolor_green, 
	   _plotter->drawstate->ps_fillcolor_blue);
  _update_buffer(&_plotter->outbuf);

  /* includes idraw directive: set fill pattern */
  if (_plotter->drawstate->fill_level == 0)	/* transparent */
    sprintf (_plotter->outbuf.current, "%%I p\nnone SetP\n");
  else	/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_plotter->outbuf.current, "%%I p\n%f SetP\n", 
	     _idraw_stdshadings[_plotter->drawstate->idraw_shading]);
  _update_buffer(&_plotter->outbuf);

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
  
  _matrix_product (offcenter_rotation_matrix, _plotter->drawstate->transform.m,
		   ellipse_transformation_matrix);
  
  /* includes idraw directive: transformation matrix (all 6 elements) */
  sprintf (_plotter->outbuf.current, "%%I t\n[");
  _update_buffer(&_plotter->outbuf);
  for (i = 0; i < 6; i++)
    {
      if ((i==0) || (i==1) || (i==2) || (i==3))
	sprintf (_plotter->outbuf.current, "%.7g ", 
		 ellipse_transformation_matrix[i] / granularity);
      else
	sprintf (_plotter->outbuf.current, "%.7g ", 
		 ellipse_transformation_matrix[i]);
      _update_buffer(&_plotter->outbuf);
    }
  sprintf (_plotter->outbuf.current, "] concat\n");
  _update_buffer(&_plotter->outbuf);

  /* includes idraw directive: draw Elli, and end Elli (or same for Circ) */
  if (circlep)
    sprintf (_plotter->outbuf.current, "%%I\n%d %d %d Circ\nEnd\n\n", 
	     IROUND(granularity * x), IROUND(granularity * y), 
	     IROUND(granularity * rx));
  else
    sprintf (_plotter->outbuf.current, "%%I\n%d %d %d %d Elli\nEnd\n\n", 
	     IROUND(granularity * x), IROUND(granularity * y), 
	     IROUND(granularity * rx), IROUND(granularity * ry));
  _update_buffer(&_plotter->outbuf);

  /* update bounding box */
  _set_ellipse_bbox (x, y, rx, ry, costheta, sintheta, 
		     _plotter->drawstate->line_width);

  _plotter->drawstate->pos.x = x;	/* move to center of ellipse */
  _plotter->drawstate->pos.y = y;

}
