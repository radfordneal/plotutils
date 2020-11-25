/* This file contains the fmiterlimit method, which is a GNU extension to
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
   and 1.0 (cuts off all miters).  In general, the miter limit is the
   cosecant of one-half of the minimum angle for mitering.  Values less
   than 1.0 are meaningless. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_fmiterlimit (R___(Plotter *_plotter) double new_miter_limit)
#else
_m_fmiterlimit (R___(_plotter) new_miter_limit)
     S___(Plotter *_plotter;)
     double new_miter_limit;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmiterlimit: invalid operation");
      return -1;
    }

  /* invoke generic method */
  _g_fmiterlimit (R___(_plotter) new_miter_limit);

  _meta_emit_byte (R___(_plotter) (int)O_FMITERLIMIT);
  _meta_emit_float (R___(_plotter) new_miter_limit);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
