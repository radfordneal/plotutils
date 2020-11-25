/* This file contains a low-level routine for repositioning the graphics
   cursor on a Tektronix display, by emitting an escape sequence.

   The reposition command automatically knocks the Tektronix into a
   non-alpha mode.  We choose either PLOT or POINT mode, depending on
   whether the polyline we're drawing is connected, or is simply a set of
   disconnected points.  That information is stored in our drawing state.

   If we are already in PLOT/POINT mode, emitting the escape sequence will
   prevent a line being drawn at the time of the move (the "dark vector"
   concept).  That is just what we want. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_tek_move (int xx, int yy)
#else
_tek_move (xx, yy)
     int xx, yy;
#endif
{
  int correct_tek_mode = 
    _plotter->drawstate->points_are_connected ? MODE_PLOT : MODE_POINT;

  switch (correct_tek_mode)
    {
    case MODE_POINT:
      /* ASCII FS, i.e. ^\ (enter POINT mode)*/
      _plotter->write_byte ('\034'); 
      break;
    case MODE_PLOT:
      /* ASCII GS, i.e. ^] (enter PLOT mode) */
      _plotter->write_byte ('\035'); 
      break;
    default:			/* shouldn't happen */
      return;
    }

  /* output location to the Tektronix */
  _tek_vector (xx, yy);

  /* Tek position is now correct */
  _plotter->tek_pos.x = xx;
  _plotter->tek_pos.y = yy;  
  _plotter->tek_position_is_unknown = false;

  /* Tek is now in correct mode for plotting vectors */
  _plotter->tek_mode_is_unknown = false;
  _plotter->tek_mode = correct_tek_mode;

  /* re-emphasize: on return we'll be in either PLOT or POINT mode. */
  return;
}
