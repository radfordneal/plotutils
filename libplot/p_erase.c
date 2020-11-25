/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   A PSPlotter simply resets the output buffer, discarding all objects
   written to it, resets the bounding box, and marks all Postscript fonts
   as unused. */

#include "sys-defines.h"
#include "plot.h"
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

  /* reinitialize `font used' array(s) */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->ps_font_used[i] = false;
#ifdef USE_LJ_FONTS
  for (i = 0; i < NUM_PCL_FONTS; i++)
    _plotter->pcl_font_used[i] = false;
#endif

  return 0;
}
