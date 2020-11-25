/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

/* This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis).
   Both methods produce output in PS format, and both call
   _p_fellipse_internal(). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_p_fcircle (R___(Plotter *_plotter) double x, double y, double radius)
#else
_p_fcircle (R___(_plotter) x, y, radius)
     S___(Plotter *_plotter;)
     double x, y, radius;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fcircle: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (!_plotter->drawstate->points_are_connected)
    /* `disconnected' line type, so don't draw anything (libplot convention) */
    {
      _plotter->drawstate->pos.x = x; /* move to center of ellipse */
      _plotter->drawstate->pos.y = y;
    }
  else
    /* final arg flags this for idraw as a circle, not an ellipse */
    _p_fellipse_internal (R___(_plotter) x, y, radius, radius, 0.0, true);

  return 0;
}

int
#ifdef _HAVE_PROTOS
_p_fellipse (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle)
#else
_p_fellipse (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;)
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fellipse: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (!_plotter->drawstate->points_are_connected)
    /* `disconnected' line type, so don't draw anything (libplot convention) */
    {
      _plotter->drawstate->pos.x = x; /* move to center of ellipse */
      _plotter->drawstate->pos.y = y;
    }
  else
    /* final arg flags this for idraw as an ellipse, not a circle */
    _p_fellipse_internal (R___(_plotter) x, y, rx, ry, angle, false);

  return 0;
}

void
#ifdef _HAVE_PROTOS
_p_fellipse_internal (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle, bool circlep)
#else
_p_fellipse_internal (R___(_plotter) x, y, rx, ry, angle, circlep)
     S___(Plotter *_plotter;)
     double x, y, rx, ry, angle;
     bool circlep;		/* drawn as a circle in user frame? */
#endif
{  
  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* have something to draw */
    {
      double granularity;
      double costheta, sintheta;
      double offcenter_rotation_matrix[6];
      double ellipse_transformation_matrix[6];
      int i;

      /* prologue instruction and idraw directive: start of Elli or Circ */
      if (circlep)
	strcpy (_plotter->page->point, "Begin %I Circ\n");
      else
	strcpy (_plotter->page->point, "Begin %I Elli\n");
      _update_buffer(_plotter->page);
      
      /* emit common attributes: CTM, fill rule, cap and join styles and
	 miter limit, dash array, foreground and background colors, and
	 idraw brush. */
      granularity = _p_emit_common_attributes (S___(_plotter));

      /* An affine tranformation must be applied to the ellipse produced by
	 the Elli routine in the idraw prologue, to turn it into the
	 ellipse we want.  The Elli routine produces an ellipse with
	 specified semi-axes, aligned parallel to the coordinate axes in
	 user space, and centered on the point (x,y).  I.e. it produces,
	 symbolically,

	 [unit circle centered on (0,0)] S T

	 where S is a diagonal matrix that scales the unit circle to give
	 the specified semi-axis lengths, and T translates (0,0) to (x,y).
	 This is not what we want, since the ellipse is not rotated (it has
	 zero inclination angle).  What we want is
 
	 [unit circle centered on (0,0)] S R T

	 where R is a rotation matrix.  This may be rewritten as
	 
	 [unit circle centered on (0,0)] S T  (T^{-1} R T)

	 where T^{-1} R T is a so-called offcenter rotation matrix, which
	 rotates about the point (x,y).  So the ellipse transformation
	 matrix we'll place in the PS code will be (T^{-1} R T) times the
	 matrix that transforms from user space to device space. */

      costheta = cos (M_PI * angle / 180.0);
      sintheta = sin (M_PI * angle / 180.0);
      
      offcenter_rotation_matrix[0] = costheta; /* 1st 4 els are those of R */
      offcenter_rotation_matrix[1] = sintheta;
      offcenter_rotation_matrix[2] = - sintheta;
      offcenter_rotation_matrix[3] = costheta;
      offcenter_rotation_matrix[4] = x * (1.0 - costheta) + y * sintheta;
      offcenter_rotation_matrix[5] = y * (1.0 - costheta) - x * sintheta;
  
      _matrix_product (offcenter_rotation_matrix,
		       _plotter->drawstate->transform.m,
		       ellipse_transformation_matrix);
  
      /* emit idraw directive: transformation matrix (all 6 elements) */
      sprintf (_plotter->page->point, "%%I t\n[");
      _update_buffer(_plotter->page);
      for (i = 0; i < 6; i++)
	{
	  if ((i==0) || (i==1) || (i==2) || (i==3))
	    sprintf (_plotter->page->point, "%.7g ", 
		     ellipse_transformation_matrix[i] / granularity);
	  else
	    sprintf (_plotter->page->point, "%.7g ", 
		     ellipse_transformation_matrix[i]);
	  _update_buffer(_plotter->page);
	}
      sprintf (_plotter->page->point, "] concat\n");
      _update_buffer(_plotter->page);
      
      /* emit idraw directive: draw Elli, and end Elli (or same for Circ) */
      if (circlep)
	sprintf (_plotter->page->point, "%%I\n%d %d %d Circ\nEnd\n\n", 
		 IROUND(granularity * x), IROUND(granularity * y), 
		 IROUND(granularity * rx));
      else
	sprintf (_plotter->page->point, "%%I\n%d %d %d %d Elli\nEnd\n\n", 
		 IROUND(granularity * x), IROUND(granularity * y), 
		 IROUND(granularity * rx), IROUND(granularity * ry));
      _update_buffer(_plotter->page);
      
      /* update bounding box */
      _set_ellipse_bbox (R___(_plotter)
			 _plotter->page, x, y, rx, ry, costheta, sintheta, 
			 _plotter->drawstate->line_width);
    }
  
  _plotter->drawstate->pos.x = x; /* move to center of ellipse */
  _plotter->drawstate->pos.y = y;

}
