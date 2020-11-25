/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_n_erase (S___(Plotter *_plotter))
#else
_n_erase (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "erase: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* delete image, i.e., deallocate libxmi canvas */
  _n_delete_image (S___(_plotter));

  /* create new image, i.e. libxmi canvas, initialized to background color */
  _n_new_image (S___(_plotter));
  
  /* on to next frame (for a PNM Plotter, which doesn't plot in real time,
     only the last frame in the page is meaningful) */
  _plotter->frame_number++;

  return 0;
}
