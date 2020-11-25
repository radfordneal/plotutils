/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   A PSPlotter simply resets the output buffer, discarding all objects
   written to it, resets the bounding box, and marks all Postscript fonts
   as unused on the current page. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_p_erase (void)
#else
_p_erase ()
#endif
{
  int i;

  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  _reset_outbuf (_plotter->page); /* discard all objects */

  /* reinitialize `font used' array(s) for this page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->page->ps_font_used[i] = false;
#ifdef USE_LJ_FONTS_IN_PS
  for (i = 0; i < NUM_PCL_FONTS; i++)
    _plotter->page->pcl_font_used[i] = false;
#endif

  /* on to next frame (for a PS Plotter, which doesn't plot in real time,
     only the last frame in the page is meaningful) */
  _plotter->frame_number++;

  return 0;
}
