/* This file contains the moverel() method, which is a GNU extension to
   libplot.  moverel() is simply a version of move() that uses relative
   coordinates.  It sets a drawing attribute: the location of the graphics
   cursor, which determines the position of the next object drawn on the
   graphics device. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_moverel (int x, int y)
#else
_m_moverel (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("moverel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_MOVEREL);
  _meta_emit_integer (x);
  _meta_emit_integer (y);
  _meta_emit_terminator ();
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmoverel (double x, double y)
#else
_m_fmoverel (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmoverel: invalid operation");
      return -1;
    }

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_MOVEREL : (int)O_FMOVEREL);
  _meta_emit_float (x);
  _meta_emit_float (y);
  _meta_emit_terminator ();
  
  return 0;
}
