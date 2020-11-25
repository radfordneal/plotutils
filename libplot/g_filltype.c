/* This file contains the filltype method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the desaturation level of the
   filling, for all objects created by the drawing operations that follow.
   (For those that can be filled, that is; text cannot be filled.)

   The argument to filltype ranges from 0 to 0xFFFF.  The value 0 is special;
   it signifies no filling at all (the object will be transparent).  The
   value 1 signifies that the fill color should be the user-specified fill
   color, and a value of 0xFFFF signifies complete desaturation of this
   color (i.e., white).  Values intermediate between 1 and 0xFFFF yield
   intermediate saturations of the user-specified fill color.  An
   out-of-bounds argument resets the desaturation level to a default
   value. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_filltype (R___(Plotter *_plotter) int level)
#else
_g_filltype (R___(_plotter) level)
     S___(Plotter *_plotter;) 
     int level;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "filltype: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if ((level < 0) || (level > 0xffff))
    /* OOB switches to default */
    level = _default_drawstate.fill_type;

  _plotter->drawstate->fill_type = level;
  
  return 0;
}
