/* This file contains the erase routine, which is a standard part of the
   libplot.  It erases the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
erase ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: erase() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any (could simply
				   interrupt it in mid-assemblage) */

  fputs ("\033\014", _outstream); /* ASCII ESC C-l, i.e. ^[^l */
  _drawstate->tek_mode = MODE_ALPHA; /* clearing screen enters alpha mode */
  _tekstate.mode = MODE_ALPHA;

  /* Note: kermit Tek emulator seems to enter graphics mode on seeing ESC
     C-l , not alpha mode.  Maybe we should specify MODE_PLOT above,
     instead of MODE_ALPHA?  The above won't hurt though, because we don't
     use MODE_ALPHA anyway (we'll have to switch away from it). */

  fflush (_outstream);
#ifdef GENUINE_TEKTRONIX
  if (_tek_display_type == D_GENERIC)
    sleep(1);			/* give storage tube time to clear */
#endif
  
  return 0;
}

