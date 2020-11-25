/* This file contains the move method, which is a standard part of libplot.
   It sets a drawing attribute: the location of the graphics cursor, which
   determines the position of the next object drawn on the graphics
   device. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_move (int x, int y)
#else
_m_move (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("move: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_MOVE);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmove (double x, double y)
#else
_m_fmove (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmove: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_MOVE : (int)O_FMOVE);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_terminator ();
  
  return 0;
}
