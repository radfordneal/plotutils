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

  _reset_buffer (&_plotter->outbuf); /* discard all objects */
  _reset_range ();		/* reset bounding box */

  /* reinitialize `font used' array */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->ps_font_used[i] = false;

  return 0;
}
