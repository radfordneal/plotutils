/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_circle (int x, int y, int r)
#else
_m_circle (x, y, r)
     int x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("circle: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_CIRCLE);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_integer (r);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcircle (double x, double y, double r)
#else
_m_fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcircle: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_CIRCLE : (int)O_FCIRCLE);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_float (r);
  _meta_emit_terminator ();
  
  return 0;
}
