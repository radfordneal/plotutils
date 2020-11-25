#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_t_erase_page (S___(Plotter *_plotter))
#else
_t_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* erase: emit ESC C-l, i.e. ^[^l */
  _write_string (_plotter->data, "\033\014");
  _plotter->tek_mode = MODE_ALPHA; /* erasing enters alpha mode */

  /* Note: kermit Tek emulator, on seeing ESC C-l , seems to enter graphics
     mode, not alpha mode.  Maybe we should specify MODE_PLOT above,
     instead of MODE_ALPHA?  The above won't hurt though, because we don't
     use MODE_ALPHA anyway (we'll have to switch away from it). */

  /* set background color (a no-op unless we're writing to a kermit
     Tektronix emulator, see t_color.c) */
  _t_set_bg_color (S___(_plotter));

  return true;
}

