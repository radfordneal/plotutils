/* This file contains the pentype method, which is a GNU extension to
   libplot.  It sets a drawing attribute: whether or not a pen should be
   used.  (Objects drawn without a pen may still be filled.) */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_pentype (R___(Plotter *_plotter) int level)
#else
_g_pentype (R___(_plotter) level)
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

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if ((level < 0) || (level > 0xffff))
    /* OOB switches to default */
    level = _default_drawstate.pen_type;

  _plotter->drawstate->pen_type = level;
  
  return 0;
}
