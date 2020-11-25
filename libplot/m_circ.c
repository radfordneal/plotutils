/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_circle (R___(Plotter *_plotter) int x, int y, int r)
#else
_m_circle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;) 
     int x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "circle: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_CIRCLE);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) r);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcircle (R___(Plotter *_plotter) double x, double y, double r)
#else
_m_fcircle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;) 
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcircle: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_CIRCLE : (int)O_FCIRCLE);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_float (R___(_plotter) r);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
