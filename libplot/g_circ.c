/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r.
   Circles are one of the three types of primitive closed path that libplot
   supports, along with boxes and ellipses. */

/* Most Plotters obviously require that the map from the user frame to the
   device frame be uniform, in order to draw a circle as a primitive (if it
   isn't, what results will be an ellipse).  The constraints on the user
   frame -> device frame map (e.g., it must be uniform) are specified by
   the internal `allowed_circle_scaling' parameter, which this code checks.  */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_fcircle (R___(Plotter *_plotter) double x, double y, double r)
#else
_API_fcircle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;)
     double x, y, r;
#endif
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcircle: invalid operation");
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

      /* begin a new path */
      _plotter->drawstate->path = _new_plPath ();

      /* place circle in path buffer */

      pc.x = x;
      pc.y = y;
      clockwise = _plotter->drawstate->orientation < 0 ? true : false;

      if ((_plotter->data->allowed_circle_scaling == AS_ANY)
	  ||
	  (_plotter->data->allowed_circle_scaling == AS_UNIFORM
	   && _plotter->drawstate->transform.uniform))
	/* place circle as a primitive, since this Plotter supports
	   drawing circles as primitives */
	_add_circle (_plotter->drawstate->path, 
			       pc, r, clockwise);
      else if ((_plotter->data->allowed_ellipse_scaling == AS_ANY)
	       ||
	       (_plotter->data->allowed_ellipse_scaling == AS_AXES_PRESERVED
		&& _plotter->drawstate->transform.axes_preserved))
	/* place circle as an ellipse, since this Plotter supports drawing
	   ellipses as primitives */
	_add_ellipse (_plotter->drawstate->path, 
				pc, r, r, 0.0, clockwise);
      else if (_plotter->data->allowed_ellarc_scaling == AS_ANY
	       || (_plotter->data->allowed_ellarc_scaling == AS_AXES_PRESERVED
		   && _plotter->drawstate->transform.axes_preserved))
	/* draw circle by placing four elliptical arcs into path buffer
	   (allowed since this Plotter supports elliptical arcs) */
	_add_circle_as_ellarcs (_plotter->drawstate->path, 
				pc, r, clockwise);
      else if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* draw circle by placing four cubic Beziers into path buffer
	   (allowed since this Plotter supports cubic Beziers) */
	_add_circle_as_bezier3s (_plotter->drawstate->path, 
				 pc, r, clockwise);
      else
	/* draw a polygonal approximation to the circle */
	_add_circle_as_lines (_plotter->drawstate->path, 
			      pc, r, clockwise);

      if (_plotter->drawstate->path->type == PATH_SEGMENT_LIST)
	/* pass all the newly added segments to the Plotter-specific
	   function maybe_paint_segments(), since some Plotters plot paths
	   in real time, i.e., prepaint them, rather than waiting until
	   endpath() is called */
	_plotter->maybe_prepaint_segments (R___(_plotter) 0);
    }

  /* move to center (libplot convention) */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  return 0;
}
