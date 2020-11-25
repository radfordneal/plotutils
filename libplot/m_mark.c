/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a plotting symbol, or marker, of specified
   type and size, at a specified location.  This symbol can be one of a
   list of standard symbols, or a single printable ASCII character in the
   current font and fontsize. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_marker (R___(Plotter *_plotter) int x, int y, int type, int size)
#else
_m_marker (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;) 
     int x, y;
     int type;
     int size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "marker: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_MARKER);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) type);
  _meta_emit_integer (R___(_plotter) size);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmarker (R___(Plotter *_plotter) double x, double y, int type, double size)
#else
_m_fmarker (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;) 
     double x, y;
     int type;
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmarker: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_MARKER : (int)O_FMARKER);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_integer (R___(_plotter) type);
  _meta_emit_float (R___(_plotter) size);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
