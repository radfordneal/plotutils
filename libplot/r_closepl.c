#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_r_end_page (S___(Plotter *_plotter))
#else
_r_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* leave ReGIS graphics mode */
  _write_string (_plotter->data, "\033\\");
  /* move terminal cursor to first line of screen, to keep from obstructing
     graphics */
  _write_string (_plotter->data, "\033[1;1H");

  return true;
}
