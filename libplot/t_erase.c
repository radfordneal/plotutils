/* This file contains the erase method, which is a standard part of the
   libplot.  It erases the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_t_erase (void)
#else
_t_erase ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any (could simply
				      interrupt it in mid-assemblage) */

  if (_plotter->outstream)
    fputs ("\033\014", _plotter->outstream); /* ASCII ESC C-l, i.e. ^[^l */
  _plotter->mode = MODE_ALPHA;	/* erase enters alpha mode */

  /* Note: kermit Tek emulator seems to enter graphics mode on seeing ESC
     C-l , not alpha mode.  Maybe we should specify MODE_PLOT above,
     instead of MODE_ALPHA?  The above won't hurt though, because we don't
     use MODE_ALPHA anyway (we'll have to switch away from it). */

  if (_plotter->outstream)
    fflush (_plotter->outstream);
#ifdef GENUINE_TEKTRONIX
  if (_plotter->display_type == D_GENERIC)
    sleep(1);			/* give storage tube time to clear */
#endif
  
  return 0;
}

