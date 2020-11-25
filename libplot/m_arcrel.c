/* This file contains the arcrel() method, which is a GNU extension to
   libplot.  arcrel() is simply a version of arc() that uses relative
   coordinates.  It draws an object: an arc with specified center,
   beginning, and end. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_arcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_arcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("arcrel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_ARCREL);
  _meta_emit_integer (xc);
  _meta_emit_integer (yc);
  _meta_emit_integer (x0);
  _meta_emit_integer (y0);
  _meta_emit_integer (x1);
  _meta_emit_integer (y1);
  _meta_emit_terminator ();

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_farcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_farcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("farcrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_ARCREL : (int)O_FARCREL);
  _meta_emit_float (xc);
  _meta_emit_float (yc);
  _meta_emit_float (x0);
  _meta_emit_float (y0);
  _meta_emit_float (x1);
  _meta_emit_float (y1);
  _meta_emit_terminator ();
  
  return 0;
}
