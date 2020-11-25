/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_arc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_arc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "arc: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ARC);
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
_m_farc (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_farc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "farc: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_ARC : (int)O_FARC);
  _meta_emit_float (R___(_plotter) xc);
  _meta_emit_float (R___(_plotter) yc);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
