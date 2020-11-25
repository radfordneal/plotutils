/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_point (int x, int y)
#else
_m_point (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("point: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_POINT);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_terminator ();
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fpoint (double x, double y)
#else
_m_fpoint (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_POINT : (int)O_FPOINT);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_terminator ();
      
  return 0;
}
