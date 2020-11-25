/* This file contains the erase method, which is a standard part of the
   libplot.  It erases the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_t_erase (S___(Plotter *_plotter))
#else
_t_erase (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "erase: invalid operation");
      return -1;
    }

  /* flush polyline if any (could simply interrupt it in mid-assemblage) */
  _plotter->endpath (S___(_plotter));

  /* erase: emit ESC C-l, i.e. ^[^l */
  _plotter->write_string (R___(_plotter) "\033\014");
  _plotter->tek_mode = MODE_ALPHA; /* erasing enters alpha mode */

  /* Note: kermit Tek emulator, on seeing ESC C-l , seems to enter graphics
     mode, not alpha mode.  Maybe we should specify MODE_PLOT above,
     instead of MODE_ALPHA?  The above won't hurt though, because we don't
     use MODE_ALPHA anyway (we'll have to switch away from it). */

  /* set background color (a no-op unless we're writing to a kermit
     Tektronix emulator, see t_color.c) */
  _plotter->set_bg_color (S___(_plotter));

  _plotter->flushpl (S___(_plotter));
#ifdef GENUINE_TEKTRONIX
  if (_plotter->tek_display_type == D_GENERIC)
    sleep(1);			/* give storage tube time to clear */
#endif
  
  /* on to next frame */
  _plotter->frame_number++;

  return 0;
}

