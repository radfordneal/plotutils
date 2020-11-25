/* This file contains the circlerel method, which is a GNU extension to
   libplot.  circlerel() is simply a version of circle() that uses relative
   coordinates.  It draws an object: a circle with specified center and
   radius. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_circlerel (int x, int y, int r)
#else
_m_circlerel (x, y, r)
     int x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("circlerel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d\n", 
		 CIRCLEREL, x, y, r);
      else
	{
	  putc (CIRCLEREL, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	  _emit_integer (r);
	}
    }

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcirclerel (double x, double y, double r)
#else
_m_fcirclerel (x, y, r)
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcirclerel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g\n", 
		 CIRCLEREL, x, y, r);
      else
	{
	  putc (FCIRCLEREL, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	  _emit_float (r);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_ellipserel (int x, int y, int rx, int ry, int angle)
#else
_m_ellipserel (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("ellipserel: invalid operation");
      return -1;
    }

  if (_plotter->portable_output)
    fprintf (_plotter->outstream, "%c %d %d %d %d %d\n", 
	     ELLIPSEREL, x, y, rx, ry, angle);
  else
    {
      putc (ELLIPSEREL, _plotter->outstream);
      _emit_integer (x);
      _emit_integer (y);
      _emit_integer (rx);
      _emit_integer (ry);
      _emit_integer (angle);
    }

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fellipserel (double x, double y, double rx, double ry, double angle)
#else
_m_fellipserel (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fellipserel: invalid operation");
      return -1;
    }

  if (_plotter->portable_output)
    fprintf (_plotter->outstream, "%c %g %g %g %g %g\n", 
	     ELLIPSEREL, x, y, rx, ry, angle);
  else
    {
      putc (FELLIPSEREL, _plotter->outstream);
      _emit_float (x);
      _emit_float (y);
      _emit_float (rx);
      _emit_float (ry);
      _emit_float (angle);
    }

  return 0;
}
