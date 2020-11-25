/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_fbox (double x0, double y0, double x1, double y1)
#else
_h_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fbox: invalid operation");
      return -1;
    }

  /* if user coors -> device coors transformation does not preserve axis
     directions, use generic method to draw a closed polyline */

  if (!_plotter->drawstate->transform.axes_preserved)
    return _g_fbox (x0, y0, x1, y1);

  /* otherwise use HP-GL's native rectangle-drawing facility, as follows */

  /* move to lower left corner, sync pen position */
  _plotter->fmove (x0, y0);
  _plotter->set_position();

  /* sync line attributes, incl. pen width */
  _plotter->set_attributes();
  
  if (_plotter->drawstate->fill_level)
    /* ideally, rectangle should be filled */
    {
      /* Sync fill color.  This may set the _plotter->bad_pen flag (e.g. if
	 optimal pen is #0 and we're not allowed to use pen #0 to draw
	 with).  So we test _plotter->bad_pen before using the pen. */
      _plotter->set_fill_color ();
      if (_plotter->bad_pen == false)
	/* fill the rectangle */
	{
	  sprintf (_plotter->page->point, "RA%d,%d;", 
		   IROUND(XD(x1,y1)), IROUND(YD(x1,y1)));
	  _update_buffer (_plotter->page);
	}
    }	  
  
  /* Sync pen color.  This may set the _plotter->bad_pen flag (e.g. if
     optimal pen is #0 and we're not allowed to use pen #0 to draw with).
     So we test _plotter->bad_pen before using the pen. */
  _plotter->set_pen_color ();
  if (_plotter->bad_pen == false)
    /* edge the rectangle */
    {
      sprintf (_plotter->page->point, "EA%d,%d;", 
	       IROUND(XD(x1,y1)), IROUND(YD(x1,y1)));
      _update_buffer (_plotter->page);
    }

  /* move to center of rectangle (libplot convention) */
  (_plotter->drawstate->pos).x = 0.5 * (x0 + x1);
  (_plotter->drawstate->pos).y = 0.5 * (y0 + y1);

  return 0;
}

