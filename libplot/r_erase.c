#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_r_erase_page (S___(Plotter *_plotter))
#else
_r_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* if necessary, update background color in internal ReGIS state */
  _r_set_bg_color (S___(_plotter));
  /* erase screen (causing background color to show up) */
  _write_string (_plotter->data, "S(E)\n");

  return true;
}


