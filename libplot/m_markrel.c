/* This file contains the markerrel method, which is a GNU extension to
   libplot.  markerrel() is simply a version of marker() that uses relative
   coordinates.  It plots an object: a plotting symbol, or marker, of
   specified type and size. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_markerrel (int x, int y, int type, int size)
#else
_m_markerrel (x, y, type, size)
     int x, y;
     int type;
     int size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("markerrel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_MARKERREL);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_integer (type);
  _meta_emit_integer (size);
  _meta_emit_terminator ();
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmarkerrel (double x, double y, int type, double size)
#else
_m_fmarkerrel (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmarkerrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_MARKERREL : (int)O_FMARKERREL);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_integer (type);
  _meta_emit_float (size);
  _meta_emit_terminator ();
  
  return 0;
}
