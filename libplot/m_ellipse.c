/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_ellipse (R___(Plotter *_plotter) int x, int y, int rx, int ry, int angle)
#else
_m_ellipse (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;) 
     int x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ellipse: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ELLIPSE);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) rx);
  _meta_emit_integer (R___(_plotter) ry);
  _meta_emit_integer (R___(_plotter) angle);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fellipse (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle)
#else
_m_fellipse (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;) 
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fellipse: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_ELLIPSE : (int)O_FELLIPSE);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_float (R___(_plotter) rx);
  _meta_emit_float (R___(_plotter) ry);
  _meta_emit_float (R___(_plotter) angle);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
