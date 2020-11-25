/* This file contains the joinmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when drawing
   subsequent polylines. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_joinmod (const char *s)
#else
_m_joinmod (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("joinmod: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_JOINMOD);
  _meta_emit_string (s);
  
  /* invoke generic method */
  return _g_joinmod (s);
}
