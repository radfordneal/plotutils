/* This file contains the circlerel method, which is a GNU extension to
   libplot.  circlerel() is simply a version of circle() that uses relative
   coordinates.  It draws an object: a circle with specified center and
   radius. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_circlerel (R___(Plotter *_plotter) int x, int y, int r)
#else
_m_circlerel (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;) 
     int x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "circlerel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_CIRCLEREL);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) r);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcirclerel (R___(Plotter *_plotter) double x, double y, double r)
#else
_m_fcirclerel (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;) 
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcirclerel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_CIRCLEREL : (int)O_FCIRCLEREL);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_float (R___(_plotter) r);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_ellipserel (R___(Plotter *_plotter) int x, int y, int rx, int ry, int angle)
#else
_m_ellipserel (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;) 
     int x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ellipserel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ELLIPSEREL);
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
_m_fellipserel (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle)
#else
_m_fellipserel (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;) 
     double x, y, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fellipserel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_ELLIPSEREL : (int)O_FELLIPSEREL);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_float (R___(_plotter) rx);
  _meta_emit_float (R___(_plotter) ry);
  _meta_emit_float (R___(_plotter) angle);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}
