/* This file contains the contrel() method, which is a GNU extension to
   libplot.  contrel() is simply a version of cont() that uses relative
   coordinates.  It continues a line. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_contrel (int x, int y)
#else
_m_contrel (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("contrel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_CONTREL);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcontrel (double x, double y)
#else
_m_fcontrel (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcontrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_CONTREL : (int)O_FCONTREL);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_terminator ();
      
  return 0;
}
