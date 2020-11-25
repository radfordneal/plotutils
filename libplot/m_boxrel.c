/* This file contains the boxrel method, which is a GNU extension to
   libplot.  boxrel() is simply a version of box() that uses relative
   coordinates.  It draws an object: an upright rectangle with specified
   diagonal corners. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_boxrel (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_m_boxrel (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "boxrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_BOXREL);
  _meta_emit_integer (R___(_plotter) x0);
  _meta_emit_integer (R___(_plotter) y0);
  _meta_emit_integer (R___(_plotter) x1);
  _meta_emit_integer (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fboxrel (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
#else
_m_fboxrel (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fboxrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_BOXREL : (int)O_FBOXREL);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
