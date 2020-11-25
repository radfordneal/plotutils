/* This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis).

   In this generic version, we draw the ellipse by drawing four elliptic
   arcs (quarter-ellipses). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fellipse (R___(Plotter *_plotter) double xc, double yc, double rx, double ry, double angle)
#else
_g_fellipse (R___(_plotter) xc, yc, rx, ry, angle)
     S___(Plotter *_plotter;)
     double xc, yc, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fellipse: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* draw only if line type isn't `disconnected' (libplot convention) */
  if (_plotter->drawstate->points_are_connected)
    {
      double theta, costheta, sintheta;

      theta = (M_PI / 180.0) * angle; /* convert to radians */
      costheta = cos (theta);
      sintheta = sin (theta);

      /* pass hint to cont() and endpath() */
      _plotter->drawstate->convex_path = true;

      _plotter->fellarc (R___(_plotter) 
			 xc, yc, 
			 xc + rx * costheta, yc + rx * sintheta,
			 xc - ry * sintheta, yc + ry * costheta);
      _plotter->fellarc (R___(_plotter) 
			 xc, yc, 
			 xc - ry * sintheta, yc + ry * costheta,
			 xc - rx * costheta, yc - rx * sintheta);
      _plotter->fellarc (R___(_plotter) 
			 xc, yc, 
			 xc - rx * costheta, yc - rx * sintheta,
			 xc + ry * sintheta, yc - ry * costheta);
      _plotter->fellarc (R___(_plotter) 
			 xc, yc, 
			 xc + ry * sintheta, yc - ry * costheta,
			 xc + rx * costheta, yc + rx * sintheta);

      /* move to center (a libplot convention); this calls endpath() to
	 flush out the polyline */
      _plotter->fmove (R___(_plotter) xc, yc);	

      _plotter->drawstate->convex_path = false; /* restore hint value */
    }
  else				/* just move to center */
    _plotter->fmove (R___(_plotter) xc, yc);	

  return 0;
}
