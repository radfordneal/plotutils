/* This file contains the move method, which is a standard part of libplot.
   It sets a drawing attribute: the location of the graphics cursor, which
   determines the position of the next object drawn on the graphics
   device. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_move (R___(Plotter *_plotter) int x, int y)
#else
_m_move (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "move: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_MOVE);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmove (R___(Plotter *_plotter) double x, double y)
#else
_m_fmove (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmove: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_MOVE : (int)O_FMOVE);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
