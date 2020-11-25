/* This file contains the bezier2rel() and bezier3rel() methods, which are
   GNU extension to libplot.  They are simply versions of bezier2() and
   bezier3() that use relative coordinates.  Each of them draws an object:
   a quadratic and a cubic Bezier path segment, respectively. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_bezier2rel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_bezier2rel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("bezier2rel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_BEZIER2REL);
  _meta_emit_integer (xc);
  _meta_emit_integer (yc);
  _meta_emit_integer (x0);
  _meta_emit_integer (y0);
  _meta_emit_integer (x1);
  _meta_emit_integer (y1);
  _meta_emit_terminator ();

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fbezier2rel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_fbezier2rel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fbezier2rel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_BEZIER2REL : (int)O_FBEZIER2REL);
  _meta_emit_float (xc);
  _meta_emit_float (yc);
  _meta_emit_float (x0);
  _meta_emit_float (y0);
  _meta_emit_float (x1);
  _meta_emit_float (y1);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_bezier3rel (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
_m_bezier3rel (x0, y0, x1, y1, x2, y2, x3, y3)
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("bezier3rel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_BEZIER3REL);
  _meta_emit_integer (x0);
  _meta_emit_integer (y0);
  _meta_emit_integer (x1);
  _meta_emit_integer (y1);
  _meta_emit_integer (x2);
  _meta_emit_integer (y2);
  _meta_emit_integer (x3);
  _meta_emit_integer (y3);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fbezier3rel (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
_m_fbezier3rel (x0, y0, x1, y1, x2, y2, x3, y3)
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fbezier3rel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_BEZIER3REL : (int)O_FBEZIER3REL);
  _meta_emit_float (x0);
  _meta_emit_float (y0);
  _meta_emit_float (x1);
  _meta_emit_float (y1);
  _meta_emit_float (x2);
  _meta_emit_float (y2);
  _meta_emit_float (x3);
  _meta_emit_float (y3);
  _meta_emit_terminator ();
  
  return 0;
}
