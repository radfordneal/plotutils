/* This file contains the fillmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the fill rule used when
   subsequently drawing filled objects, i.e., the rule used to determine
   which points are `inside'.

   In principle, both the `odd winding number' rule and the `nonzero
   winding number' rule are supported.  The former is the default. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_fillmod (R___(Plotter *_plotter) const char *s)
#else
_m_fillmod (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fillmod: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_FILLMOD);
  _meta_emit_string (R___(_plotter) s);
  
  /* invoke generic method */
  return _g_fillmod (R___(_plotter) s);
}
