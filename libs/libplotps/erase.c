/* This file contains the erase routine, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   For libplotps, we discard all objects written to the output buffer. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
erase ()
{
  int i;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: erase() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _reset_buffer (&_outbuf);	/* discard all objects */
  _reset_range ();		/* reset bounding box */

  /* mark all PS fonts as unused */
  i = -1;
  while (_ps_font_info[++i].ps_name)
    _ps_font_info[i].used = FALSE;

  return 0;
}
