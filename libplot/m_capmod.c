/* This file contains the capmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the cap mode used when drawing
   subsequent lines and polylines. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_capmod (const char *s)
#else
_m_capmod (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("capmod: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_CAPMOD);
  _meta_emit_string (s);
  
  /* invoke generic method */
  return _g_capmod (s);
}
