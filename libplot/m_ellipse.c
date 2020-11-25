/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
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

  _meta_emit_byte ((int)O_ELLIPSE);
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

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_ELLIPSE : (int)O_FELLIPSE);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_float (rx);
  _meta_emit_float (ry);
  _meta_emit_float (angle);
  _meta_emit_terminator ();
  
  return 0;
}
