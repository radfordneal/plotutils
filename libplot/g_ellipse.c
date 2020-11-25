/* This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis).
   Ellipses are one of the three types of primitive closed path that
   libplot supports, along with boxes and circles.

   In this generic version, we usually draw the ellipse by drawing four
   elliptic arcs (quarter-ellipses) into the path buffer's segment list.
   Plotters that don't support elliptic arcs draw polygonal approximations
   instead.  Plotters that are sufficiently powerful that they support
   ellipses as primitive drawing elements place an ellipse primitive in the
   path buffer, instead.

   Note that some Plotters support the drawing of any ellipse; some require
   that they be aligned with the coordinate axes.  Some don't support the
   drawing of ellipses at all.  The constraints on the user frame -> device
   frame map (e.g., it must preserve coordinate axes) are specified by the
   internal `allowed_ellipse_scaling' parameter, which this code checks. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_fellipse (R___(Plotter *_plotter) double xc, double yc, double rx, double ry, double angle)
#else
_API_fellipse (R___(_plotter) xc, yc, rx, ry, angle)
     S___(Plotter *_plotter;)
     double xc, yc, rx, ry, angle;
#endif
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fellipse: invalid operation");
      return -1;
    }

  /* If a simple path is under construction (so that endsubpath() must not
      have been invoked), flush out the whole compound path.  (It may
      include other, previously drawn simple paths.) */
  if (_plotter->drawstate->path)
    _API_endpath (S___(_plotter));

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
    }

  else
    /* general case */
    {
      plPoint pc;
      bool clockwise;

      /* determine whether ellipse's axes are aligned with the coordinate
	 axes in the user frame, so that (if the device->user frame map
	 preserves axes) the same will be true in the device frame */
      bool aligned_ellipse = false;
      int iangle = IROUND(angle);
	
      if (iangle < 0)
	iangle +=  (1 + (-iangle / 90)) * 90;
      if (iangle % 90 == 0 && angle == (double)iangle)
	aligned_ellipse = true;
      
      /* begin a new path */
      _plotter->drawstate->path = _new_plPath ();

      /* place ellipse in path buffer */

      pc.x = xc;
      pc.y = yc;
      clockwise = _plotter->drawstate->orientation < 0 ? true : false;

      if ((_plotter->data->allowed_ellipse_scaling == AS_ANY)
	  ||
	  (_plotter->data->allowed_ellipse_scaling == AS_AXES_PRESERVED
	   && _plotter->drawstate->transform.axes_preserved
	   && aligned_ellipse))
	/* place ellipse as a primitive, since this Plotter supports
	   drawing ellipses as primitives */
	_add_ellipse (_plotter->drawstate->path, 
		      pc, rx, ry, angle, clockwise);

      else if (_plotter->data->allowed_ellarc_scaling == AS_ANY
	       || (_plotter->data->allowed_ellarc_scaling == AS_AXES_PRESERVED
		   && _plotter->drawstate->transform.axes_preserved
		   && aligned_ellipse))
	/* draw ellipse by placing four elliptic arcs into path buffer
	   (allowed since this Plotter supports elliptic arcs) */
	_add_ellipse_as_ellarcs (_plotter->drawstate->path, 
				 pc, rx, ry, angle, clockwise);

      else if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* draw ellipse by placing four cubic Beziers into path buffer
	   (allowed since this Plotter supports cubic Beziers) */
	_add_ellipse_as_bezier3s (_plotter->drawstate->path, 
				  pc, rx, ry, angle, clockwise);
      else
	/* draw a polygonal approximation to the ellipse */
	_add_ellipse_as_lines (_plotter->drawstate->path, 
			       pc, rx, ry, angle, clockwise);

      if (_plotter->drawstate->path->type == PATH_SEGMENT_LIST)
	/* pass all the newly added segments to the Plotter-specific
	   function maybe_paint_segments(), since some Plotters plot paths
	   in real time, i.e., prepaint them, rather than waiting until
	   endpath() is called */
	_plotter->maybe_prepaint_segments (R___(_plotter) 0);
    }

  /* move to center (libplot convention) */
  _plotter->drawstate->pos.x = xc;
  _plotter->drawstate->pos.y = yc;

  return 0;
}
