/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_n_erase (void)
#else
_n_erase ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* deallocate bitmap */
  _n_delete_image ();

  /* create new image, i.e. bitmap, initialized to background color */
  _n_new_image ();
  
  /* on to next frame (for a PNM Plotter, which doesn't plot in real time,
     only the last frame in the page is meaningful) */
  _plotter->frame_number++;

  return 0;
}
