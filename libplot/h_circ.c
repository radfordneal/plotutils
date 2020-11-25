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

  (_plotter->drawstate->pos).x = x; /* move to center (a libplot convention) */
  (_plotter->drawstate->pos).y = y;
  
  radius = sqrt(XDV(r,0)*XDV(r,0)+YDV(r,0)*YDV(r,0));
  
  /* sync pen position and line attributes, incl. pen width */
  _plotter->set_position();
  _plotter->set_attributes();
  
  if (_plotter->hpgl_version >= 1)
    /* have a polygon buffer, and will use it */
    {
      /* enter polygon mode, draw circle, exit polygon mode */
      if (_plotter->hpgl_version == 1)
	sprintf (_plotter->outbuf.current, "PM0;CI%d;PM2;", IROUND(radius));
      else			/* HP-GL/2, use small chord angle */
	sprintf (_plotter->outbuf.current, "PM0;CI%d,2;PM2;", IROUND(radius));
      _update_buffer (&_plotter->outbuf);
      /* lift pen */
      strcpy (_plotter->outbuf.current, "PU;");
      _update_buffer (&_plotter->outbuf);
      _plotter->pendown = false;
      if (_plotter->drawstate->fill_level)
	{
	  /* select appropriate pen and fill the circle */
	  _plotter->set_fill_color ();
	  strcpy (_plotter->outbuf.current, "FP;");
	  _update_buffer (&_plotter->outbuf);
	}
      /* select appropriate pen and edge the circle */
      _plotter->set_pen_color ();
      strcpy (_plotter->outbuf.current, "EP;");
      _update_buffer (&_plotter->outbuf);
    }
  else
    /* don't have a polygon buffer, won't do filling */
    {
      /* select appropriate pen */
      _plotter->set_pen_color ();
      /* draw circle */
      sprintf (_plotter->outbuf.current, "CI%d;", IROUND(radius));
      _update_buffer (&_plotter->outbuf);
    }
  
  return 0;
}
