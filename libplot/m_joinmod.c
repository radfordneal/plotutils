/* This file contains the joinmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when drawing
   subsequent polylines. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_joinmod (R___(Plotter *_plotter) const char *s)
#else
_m_joinmod (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "joinmod: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_JOINMOD);
  _meta_emit_string (R___(_plotter) s);
  
  /* invoke generic method */
  return _g_joinmod (R___(_plotter) s);
}
