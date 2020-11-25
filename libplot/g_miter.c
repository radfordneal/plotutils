/* This file contains the miterlimit method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the miter limit of polylines
   subsequently drawn on the display device.  This attribute controls the
   treatment of corners when the join mode is set to "miter".

   At a join point of a wide polyline, the `miter length' is defined to be
   the distance between the inner corner and the outer corner.  The miter
   limit is the maximum value that can be tolerated for the miter length
   divided by the line width.  If this value is exceeded, the miter will be
   `cut off': the "bevel" join mode will be used instead.

   Examples of typical values for the miter limit are 10.43 (the
   unchangeable value used by the X Window System, which cuts off miters at
   join angles less than 11 degrees), 5.0 (the default value used by
   HP-GL/2 and PCL 5 devices, which cuts off miters at join angles less
   than 22.1 degrees), 2.0 (cuts off miters at join angles less than 60
   degrees), 1.414 (cuts off miters at join angles less than 90 degrees),
   and 1.0 (cuts off all miters).  

   In general, the miter limit is the cosecant of one-half of the minimum
   join angle for mitering, and values less than 1.0 are meaningless.  For
   example, 10.43 is csc((11 degrees)/2) = 1 / sin((11 degrees)/2).
   Mitering is allowed to take place if 1 / sin(theta/2) <= MITERLIMIT,
   i.e. sin(theta/2) >= 1/MITERLIMIT, where theta > 0 is the join angle. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fmiterlimit (double new_miter_limit)
#else
_g_fmiterlimit (new_miter_limit)
     double new_miter_limit;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("flinewidth: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  if (new_miter_limit < 1.0)	/* reset to default */
    new_miter_limit = DEFAULT_MITER_LIMIT;

  /* set the new miter limit in the drawing state */
  _plotter->drawstate->miter_limit = new_miter_limit;
  
  return 0;
}
