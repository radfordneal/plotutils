/* This file contains the bezier2rel() and bezier3rel() methods, which are
   GNU extension to libplot.  They are simply versions of bezier2() and
   bezier3() that use relative coordinates.  Each of them draws an object:
   a quadratic and a cubic Bezier path segment, respectively. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_bezier2rel (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_bezier2rel (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "bezier2rel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_BEZIER2REL);
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
_m_fbezier2rel (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_fbezier2rel (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbezier2rel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_BEZIER2REL : (int)O_FBEZIER2REL);
  _meta_emit_float (R___(_plotter) xc);
  _meta_emit_float (R___(_plotter) yc);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_bezier3rel (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
_m_bezier3rel (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;)
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "bezier3rel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_BEZIER3REL);
  _meta_emit_integer (R___(_plotter) x0);
  _meta_emit_integer (R___(_plotter) y0);
  _meta_emit_integer (R___(_plotter) x1);
  _meta_emit_integer (R___(_plotter) y1);
  _meta_emit_integer (R___(_plotter) x2);
  _meta_emit_integer (R___(_plotter) y2);
  _meta_emit_integer (R___(_plotter) x3);
  _meta_emit_integer (R___(_plotter) y3);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fbezier3rel (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
_m_fbezier3rel (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbezier3rel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_BEZIER3REL : (int)O_FBEZIER3REL);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_float (R___(_plotter) x2);
  _meta_emit_float (R___(_plotter) y2);
  _meta_emit_float (R___(_plotter) x3);
  _meta_emit_float (R___(_plotter) y3);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
