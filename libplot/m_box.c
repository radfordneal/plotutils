/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_box (int x0, int y0, int x1, int y1)
#else
_m_box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("box: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_BOX);
  _meta_emit_integer (x0);
  _meta_emit_integer (y0);
  _meta_emit_integer (x1);
  _meta_emit_integer (y1);
  _meta_emit_terminator ();

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fbox (double x0, double y0, double x1, double y1)
#else
_m_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fbox: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_BOX : (int)O_FBOX);
  _meta_emit_float (x0);
  _meta_emit_float (y0);
  _meta_emit_float (x1);
  _meta_emit_float (y1);
  _meta_emit_terminator ();
  
  return 0;
}
