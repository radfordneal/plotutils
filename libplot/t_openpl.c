/* This version is for Tek Plotters.  We emit terminal initialization
   codes, depending on the Tektronix type. */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_t_begin_page (S___(Plotter *_plotter))
#else
_t_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* send graphics initialization commands to output stream if necessary */

  if (_plotter->tek_display_type == D_XTERM 
      || _plotter->tek_display_type == D_KERMIT)
    /* VT340 command, put xterm / kermit into Tek mode */
    _write_string (_plotter->data, "\033[?38h");

  return true;
}
