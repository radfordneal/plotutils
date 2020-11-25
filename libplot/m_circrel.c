/* This file contains the circlerel method, which is a GNU extension to
   libplot.  circlerel() is simply a version of circle() that uses relative
   coordinates.  It draws an object: a circle with specified center and
   radius. */

#include "sys-defines.h"
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

  _meta_emit_byte ((int)O_CIRCLEREL);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_integer (r);
  _meta_emit_terminator ();

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

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_CIRCLEREL : (int)O_FCIRCLEREL);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_float (r);
  _meta_emit_terminator ();
  
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

  _meta_emit_byte ((int)O_ELLIPSEREL);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_integer (rx);
  _meta_emit_integer (ry);
  _meta_emit_integer (angle);
  _meta_emit_terminator ();

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

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_ELLIPSEREL : (int)O_FELLIPSEREL);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_float (rx);
  _meta_emit_float (ry);
  _meta_emit_float (angle);
  _meta_emit_terminator ();

  return 0;
}
