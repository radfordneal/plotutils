/* This file contains the linerel() method, which is a GNU extension to
   libplot.  linerel() is simply a version of line() that uses relative
   coordinates.  It draws an object: a line segment extending from one
   specified point to another.  By repeatedly invoking cont(), the user may
   extend this line object to a polyline object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linerel (int x0, int y0, int x1, int y1)
#else
_m_linerel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("linerel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_LINEREL);
  _meta_emit_integer (x0);
  _meta_emit_integer (y0);
  _meta_emit_integer (x1);
  _meta_emit_integer (y1);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_flinerel (double x0, double y0, double x1, double y1)
#else
_m_flinerel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("flinerel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_LINEREL : (int)O_FLINEREL);
  _meta_emit_float (x0);
  _meta_emit_float (y0);
  _meta_emit_float (x1);
  _meta_emit_float (y1);
  _meta_emit_terminator ();
  
  return 0;
}
