/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a plotting symbol, or marker, of specified
   type and size, at a specified location.  This symbol can be one of a
   list of standard symbols, or a single printable ASCII character in the
   current font and fontsize. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_marker (int x, int y, int type, int size)
#else
_m_marker (x, y, type, size)
     int x, y;
     int type;
     int size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("marker: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_MARKER);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_integer (type);
  _meta_emit_integer (size);
  _meta_emit_terminator ();
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmarker (double x, double y, int type, double size)
#else
_m_fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmarker: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_MARKER : (int)O_FMARKER);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_integer (type);
  _meta_emit_float (size);
  _meta_emit_terminator ();
  
  return 0;
}
