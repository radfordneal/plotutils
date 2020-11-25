/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_erase (void)
#else
_m_erase ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_ERASE);
  _meta_emit_terminator ();

  /* on to next frame */
  _plotter->frame_number++;

  return 0;
}
