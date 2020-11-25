/* This file contains the pointrel method, which is a GNU extension to
   libplot.  pointrel() is simply a version of point() that uses relative
   coordinates.  It plots an object: a point with specified coordinates. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_pointrel (int x, int y)
#else
_m_pointrel (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("pointrel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_POINTREL);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_terminator ();
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fpointrel (double x, double y)
#else
_m_fpointrel (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fpointrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_POINTREL : (int)O_FPOINTREL);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_terminator ();
      
  return 0;
}

