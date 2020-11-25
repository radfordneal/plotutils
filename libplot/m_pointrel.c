/* This file contains the pointrel method, which is a GNU extension to
   libplot.  pointrel() is simply a version of point() that uses relative
   coordinates.  It plots an object: a point with specified coordinates. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_pointrel (R___(Plotter *_plotter) int x, int y)
#else
_m_pointrel (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "pointrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_POINTREL);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fpointrel (R___(Plotter *_plotter) double x, double y)
#else
_m_fpointrel (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fpointrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_POINTREL : (int)O_FPOINTREL);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}

