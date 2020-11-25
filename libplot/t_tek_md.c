/* This file contains a low-level routine for shifting a Tektronix graphics
   device to a specified display mode, by emitting an escape sequence if
   necessary. */

/* Basic Tektronix modes are alpha mode (for drawing alphanumeric
   characters in any of four character sizes), ``graph mode'' i.e. vector
   plot mode (for drawing line segments), and GIN (graphics input) mode,
   which requests a cursor location from the user.  The EGM (enhanced
   graphics module), besides increasing the addressable point from 0..1023
   x 0..1023 to 0..4095 x 0..4095, added three additional modes: point plot
   mode, special point plot mode, and incremental plot mode.

   Switching among the three basic modes is accomplished thus:

   US, CR, or ESC FF: enter alpha mode (CR repositions cursor to left edge,
   	and ESC FF homes it)
   GS: enter graph mode
   ESC ENQ: enquire cursor location (works in both alpha, graph mode),
	i.e. temporarily enter GIN mode and back out.  In alpha mode
	cursor location is lower left corner of flickerint 8x9 cursor.
   ESC SUB: enter GIN mode, turn on positionable crosshair cursor
        and send address in response to ESC ENQ.  Thumbwheels 
	(potentiometers located on the keyboard) were used for positioning. 

   Getting back to alpha / graph mode from GIN is complicated; it's
   best to send a US or GS to ensure this happens.

   A genuine 4014 doesn't normally plot the first vector drawn after graph
   mode is entered via GS (it's ``dark vector'').  To get it plotted, you
   need to issue a BEL, VT, HT, LF, CR or BS immediately after issuing the
   GS.  Only the first of these (BEL) won't disturb the location of the
   graphics cursor.

   If the EGM is present, we also have:

   FS: enter point plot mode
   RS: enter incremental mode
   ESC FS: enter special point plot mode
   
   Point plot commands are identical to vector plot commands (the endpoint
   of the vector is drawn).  Incremental plot mode allows motion by one
   unit in any of 8 directions, on receipt of a single byte.  First byte
   after RS must be a beam-off or beam-on command.

   In special point plot mode, the byte after the ESC FS (and the byte
   which precedes every point address thereafter) is interpreted as an
   intensity character, which determines the on-time of the beam.  It is
   loaded into an intensity register.  This allows control of the dot size
   (focused vs. unfocused), and brightness, in any of these three new
   modes. 

   The user can also control the trichotomy normal z-axis (i.e. focused
   beam) / defocused z-axis (i.e. defocused beam) / write-thru mode (see
   tek2plot.c for the control codes used for this).  Write-thru simply
   means that written data, whether alpha characters or vectors, are
   written to screen but are not refreshed.  So they appear briefly and
   then vanish (unless they are refreshed under user control, by drawing
   them repeatedly).

   WARNING: it is a peculiarity of the 4014 that in the following list,
   one can mode-switch only _downward_, not upward!  The exception is
   that one can always switch up to alpha mode.
   
   	alpha mode
	vector mode
	point plot mode
	special point plot mode
	incremental plot mode.
	
   Control codes that would switch `upward' are ignored.  GIN mode can be
   switched into from any mode, however.
*/

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_tek_mode(R___(Plotter *_plotter) int newmode)
#else
_tek_mode(R___(_plotter) newmode)
     S___(Plotter *_plotter;)
     int newmode;
#endif
{
  if (_plotter->tek_mode_is_unknown || _plotter->tek_mode != newmode)
    /* need to emit escape sequence */
    {
      switch (newmode)
	{
	case MODE_ALPHA:
	  /* ASCII US, i.e. ^_ (enter alpha mode) */
	  _plotter->write_byte (R___(_plotter) '\037');
	  break;
	case MODE_PLOT:
	  if ((_plotter->tek_mode_is_unknown) 
	      || (_plotter->tek_mode == MODE_POINT)
	      || (_plotter->tek_mode == MODE_INCREMENTAL))
	    /* ASCII US, i.e. ^_ (enter alpha) */
	    _plotter->write_byte (R___(_plotter) '\037');
	  /* ASCII GS, i.e. ^] (enter vector mode)*/
	  _plotter->write_byte (R___(_plotter) '\035');
	  break;
	case MODE_POINT:
	  if ((_plotter->tek_mode_is_unknown) || 
	      (_plotter->tek_mode == MODE_INCREMENTAL))
	    /* ASCII US, i.e. ^_ (enter alpha) */
	    _plotter->write_byte (R___(_plotter) '\037'); 
	  /* ASCII FS, i.e. ^\ (enter point mode) */
	  _plotter->write_byte (R___(_plotter) '\034'); 
	  break;
	case MODE_INCREMENTAL:
	  /* ASCII RS, i.e. ^^ (enter incplot mode)*/
	  _plotter->write_byte (R___(_plotter) '\036'); 
	  break;
	default:
	  break;
	}

      /* Tektronix is now in specified internal state */
      _plotter->tek_mode = newmode;
      _plotter->tek_mode_is_unknown = false;
    }
}
