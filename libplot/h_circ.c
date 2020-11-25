/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_fcircle (R___(Plotter *_plotter) double x, double y, double r)
#else
_h_fcircle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;)
     double x, y, r;
#endif
{
  double radius;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fcircle: invalid operation");
      return -1;
    }

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
      /* just move to center (libplot convention), flushing any polyline */
      _plotter->fmove (R___(_plotter) x, y);
      return 0;
    }

  /* if affine map from user frame to device frame is anisotropic, use
     generic class method to draw an inscribed polyline */
  if (!_plotter->drawstate->transform.uniform)
    return _g_fcircle (R___(_plotter) x, y, r);
  
  /* otherwise use HP-GL's native circle-drawing facility, as follows */

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* something to draw */
    {
      radius = sqrt(XDV(r,0)*XDV(r,0)+YDV(r,0)*YDV(r,0));
      
      /* sync attributes, incl. pen width; move to center of circle */
      _plotter->set_attributes (S___(_plotter));
      _plotter->drawstate->pos.x = x;
      _plotter->drawstate->pos.y = y;
      _plotter->set_position (S___(_plotter));
      
      if (_plotter->drawstate->fill_type)
	/* circle should be filled */
	{
	  /* Sync fill color.  This may set the _plotter->hpgl_bad_pen flag
	     (e.g. if optimal pen is #0 [white] and we're not allowed to
	     use pen #0 to draw with).  So we test _plotter->hpgl_bad_pen
	     before using the pen. */
	  _plotter->set_fill_color (S___(_plotter));
	  if (_plotter->hpgl_bad_pen == false)
	    /* fill the circle (360 degree wedge) */
	    {
	      sprintf (_plotter->page->point, "WG%d,0,360;", IROUND(radius));
	      _update_buffer (_plotter->page);
	    }
	}

      if (_plotter->drawstate->pen_type)
	/* circle should be edged */
	{
	  /* Sync pen color.  This may set the _plotter->hpgl_bad_pen flag
	     (e.g. if optimal pen is #0 [white] and we're not allowed to
	     use pen #0 to draw with).  So we test _plotter->hpgl_bad_pen
	     before using the pen. */
	  _plotter->set_pen_color (S___(_plotter));
	  if (_plotter->hpgl_bad_pen == false)
	    /* do the edging */
	    {
	      sprintf (_plotter->page->point, "CI%d;", IROUND(radius));
	      _update_buffer (_plotter->page);
	    }
	}
    }

  return 0;
}
