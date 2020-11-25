/* This file contains a low-level routine for synchronizing the graphics
   cursor position of a Tektronix device with libplottek's notion of what
   the position ought to be.

   The command to reposition the cursor automatically knocks the Tektronix
   into a non-alpha mode.  We choose either PLOT or POINT mode, depending
   on whether the polylines we're drawing are connected, or are simply
   disconnected points.  That information is stored in our drawing state.

   Note that if we are already in PLOT/POINT mode, outputing another escape
   sequence, as we do, will prevent a line being drawn at the time of the
   move.  That is of course what we want. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
_tek_move (xx, yy, force)
     int xx, yy;
     Boolean force;		/* force emitting of mode-switch command */
{
  int correct_tek_mode = _drawstate->points_are_connected ? MODE_PLOT : MODE_POINT;

  if (force 
      || _tekstate.mode_is_unknown || (_tekstate.mode != correct_tek_mode))
    /* emit mode-switching sequence */
    {
      switch (correct_tek_mode)
	{
	case MODE_POINT:
	  putc ('\034', _outstream); /*ASCII FS, i.e. ^\ (enter POINT mode)*/
	  break;
	case MODE_PLOT:
	  putc ('\035', _outstream); /*ASCII GS, i.e. ^] (enter PLOT mode) */
	  break;
	default:			/* shouldn't happen */
	  return;
	}
    }

  /* output location to the Tektronix */
  _putcode (xx, yy, _outstream);

  /* Tek position is now correct */
  _tekstate.pos.x = xx;
  _tekstate.pos.y = yy;  
  _tekstate.position_is_unknown = FALSE;

  /* Tek plotting mode is now correct */
  _tekstate.mode_is_unknown = FALSE;
  _tekstate.mode = correct_tek_mode;
  _drawstate->tek_mode = correct_tek_mode; /* update our notion of mode */

  /* re-emphasize: on return we'll be in either PLOT or POINT mode. */
  return;
}

