/* This file contains the contrel() method, which is a GNU extension to
   libplot.  contrel() is simply a version of cont() that uses relative
   coordinates.  It continues a line. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_contrel (R___(Plotter *_plotter) int x, int y)
#else
_m_contrel (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "contrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_CONTREL);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcontrel (R___(Plotter *_plotter) double x, double y)
#else
_m_fcontrel (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcontrel: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_CONTREL : (int)O_FCONTREL);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}
