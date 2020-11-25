/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_erase (S___(Plotter *_plotter))
#else
_m_erase (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "erase: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ERASE);
  _meta_emit_terminator (S___(_plotter));

  /* on to next frame */
  _plotter->frame_number++;

  return 0;
}
