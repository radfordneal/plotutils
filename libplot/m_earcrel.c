/* This file contains the ellarcrel() method, which is a GNU extension to
   libplot.  ellarcrel() is simply a version of ellarc() that uses relative
   coordinates.  It draws an object: a counterclockwise arc with specified
   center, beginning, and end. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_ellarcrel (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_ellarcrel (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ellarcrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ELLARCREL);
  _meta_emit_integer (R___(_plotter) xc);
  _meta_emit_integer (R___(_plotter) yc);
  _meta_emit_integer (R___(_plotter) x0);
  _meta_emit_integer (R___(_plotter) y0);
  _meta_emit_integer (R___(_plotter) x1);
  _meta_emit_integer (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fellarcrel (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_fellarcrel (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fellarcrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_ELLARCREL : (int)O_FELLARCREL);
  _meta_emit_float (R___(_plotter) xc);
  _meta_emit_float (R___(_plotter) yc);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}
