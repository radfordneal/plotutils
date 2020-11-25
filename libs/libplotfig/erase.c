/* This file contains the erase routine, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   For libplotfig, we reset the output buffer, discarding all objects and
   all user-defined colors.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
erase ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: erase() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _reset_buffer (&_outbuf);

  _num_usercolors = 0;		/* forget user-defined colors */
  
  /* reset these fields too */
  _drawstate->fig_drawing_depth = _default_drawstate.fig_drawing_depth;
  _drawstate->fig_last_priority = _default_drawstate.fig_last_priority;  

  return 0;
}
