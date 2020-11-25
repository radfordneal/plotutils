/* This file contains the markerrel method, which is a GNU extension to
   libplot.  markerrel() is simply a version of marker() that uses relative
   coordinates.  It plots an object: a plotting symbol, or marker, of
   specified type and size. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_markerrel (R___(Plotter *_plotter) int x, int y, int type, int size)
#else
_m_markerrel (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;) 
     int x, y;
     int type;
     int size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "markerrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_MARKERREL);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) type);
  _meta_emit_integer (R___(_plotter) size);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmarkerrel (R___(Plotter *_plotter) double x, double y, int type, double size)
#else
_m_fmarkerrel (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;) 
     double x, y;
     int type;
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmarkerrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_MARKERREL : (int)O_FMARKERREL);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) type);
  _meta_emit_float (R___(_plotter) size);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
