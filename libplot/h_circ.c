/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_fcircle (double x, double y, double r)
#else
_h_fcircle (x, y, r)
     double x, y, r;
#endif
{
  double radius;

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing */
    {
      _plotter->endpath (); /* flush polyline if any */
      return 0;
    }

  /* if affine map from user frame to device frame is anisotropic, use
     generic class method to draw an inscribed polyline */
  if (!_plotter->drawstate->transform.uniform)
    return _g_fcircle (x, y, r);
  
  if (!_plotter->open)
    {
      _plotter->error ("fcircle: invalid operation");
      return -1;
    }

  /* otherwise use HP-GL's native circle-drawing facility, as follows */

  _plotter->endpath (); /* flush polyline if any */

  radius = sqrt(XDV(r,0)*XDV(r,0)+YDV(r,0)*YDV(r,0));
  
  /* move to center; set attributes, incl. pen width */
  (_plotter->drawstate->pos).x = x;
  (_plotter->drawstate->pos).y = y;
  _plotter->set_position();
  _plotter->set_attributes();
  
  if (_plotter->drawstate->fill_level)
    /* ideally, circle should be filled */
    {
      /* Sync fill color.  This may set the _plotter->bad_pen flag (e.g. if
	 optimal pen is #0 and we're not allowed to use pen #0 to draw
	 with).  So we test _plotter->bad_pen before using the pen. */
      _plotter->set_fill_color ();
      if (_plotter->bad_pen == false)
	/* fill the circle (360 degree wedge) */
	{
	  sprintf (_plotter->page->point, "WG%d,0,360;", IROUND(radius));
	  _update_buffer (_plotter->page);
	}
    }

  /* Sync pen color.  This may set the _plotter->bad_pen flag (e.g. if
     optimal pen is #0 and we're not allowed to use pen #0 to draw with).
     So we test _plotter->bad_pen before using the pen. */
  _plotter->set_pen_color ();
  if (_plotter->bad_pen == false)
    /* edge the circle */
    {
      sprintf (_plotter->page->point, "CI%d;", IROUND(radius));
      _update_buffer (_plotter->page);
    }

  return 0;
}
