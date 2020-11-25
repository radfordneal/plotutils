/* This file contains the pentype method, which is a GNU extension to
   libplot.  It sets a drawing attribute: whether or not a pen should be
   used.  (Objects drawn without a pen may still be filled.) */

#include "sys-defines.h"
#include "extern.h" 

int
#ifdef _HAVE_PROTOS
_m_pentype (R___(Plotter *_plotter) int level)
#else
_m_pentype (R___(_plotter) level)
     S___(Plotter *_plotter;) 
     int level;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "pentype: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_PENTYPE);
  _meta_emit_integer (R___(_plotter) level);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_pentype (R___(_plotter) level);
}
