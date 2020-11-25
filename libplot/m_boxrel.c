/* This file contains the boxrel method, which is a GNU extension to
   libplot.  boxrel() is simply a version of box() that uses relative
   coordinates.  It draws an object: an upright rectangle with specified
   diagonal corners. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_boxrel (int x0, int y0, int x1, int y1)
#else
_m_boxrel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("boxrel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_BOXREL);
  _meta_emit_integer (x0);
  _meta_emit_integer (y0);
  _meta_emit_integer (x1);
  _meta_emit_integer (y1);
  _meta_emit_terminator ();

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fboxrel (double x0, double y0, double x1, double y1)
#else
_m_fboxrel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fboxrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_BOXREL : (int)O_FBOXREL);
  _meta_emit_float (x0);
  _meta_emit_float (y0);
  _meta_emit_float (x1);
  _meta_emit_float (y1);
  _meta_emit_terminator ();
  
  return 0;
}
