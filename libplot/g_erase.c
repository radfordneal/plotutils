/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   This generic version is designed mostly for Plotters that do not do
   real-time plotting, and have no internal state.  It simply resets the
   output buffer for the current `page', discarding all objects written 
   to it. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_erase (S___(Plotter *_plotter))
#else
_g_erase (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "erase: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* if we have an output buffer, i.e., if we're not plotting in real time,
     discard all objects */
  if (_plotter->page)
    _reset_outbuf (_plotter->page);

  /* on to next frame */
  _plotter->frame_number++;

  return 0;
}
