/* This version is for ReGIS Plotters. */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_r_begin_page (S___(Plotter *_plotter))
#else
_r_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* send graphics initialization commands to output stream */

  /* clear terminal screen */
  _write_string (_plotter->data, "\033[2J");
  /* enter ReGIS graphics */
  _write_string (_plotter->data, "\033P1p");
  /* turn off graphics cursor */
  _write_string (_plotter->data, "S(C0)\n");

  /* copy libplot's background color to internal ReGIS state */
  _r_set_bg_color (S___(_plotter));
  /* erase screen (causing background color to show up) */
  _write_string (_plotter->data, "S(E)\n");

  return true;
}
