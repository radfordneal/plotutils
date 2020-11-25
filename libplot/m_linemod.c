/* This file contains the linemod method, which is a standard part of
   libplot.  It sets a drawing attribute: the line style used in subsequent
   drawing operations. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linemod (const char *s)
#else
_m_linemod (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("linemod: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_LINEMOD);
  _meta_emit_string (s);
  
  /* invoke generic method */
  return _g_linemod (s);
}
