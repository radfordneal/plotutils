/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_point (R___(Plotter *_plotter) int x, int y)
#else
_m_point (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "point: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_POINT);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fpoint (R___(Plotter *_plotter) double x, double y)
#else
_m_fpoint (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fpoint: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_POINT : (int)O_FPOINT);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}
