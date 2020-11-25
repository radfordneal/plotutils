/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_ellipse (int x, int y, int rx, int ry, int angle)
#else
_m_ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("ellipse: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d %d\n", 
		 ELLIPSE, x, y, rx, ry, angle);
      else
	{
	  putc (ELLIPSE, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	  _emit_integer (rx);
      _emit_integer (ry);
	  _emit_integer (angle);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fellipse (double x, double y, double rx, double ry, double angle)
#else
_m_fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g %g %g\n", 
		 ELLIPSE, x, y, rx, ry, angle);
      else
	{
	  putc (FELLIPSE, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	  _emit_float (rx);
	  _emit_float (ry);
	  _emit_float (angle);
	}
    }
  
  return 0;
}
