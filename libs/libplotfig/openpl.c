/* This file contains the openpl routine, which is a standard part of
   libplot.  It opens the graphics device.

   For libplotfig, we print out the fig header, and initialize the buffer
   in which we'll store all genuine objects.  There is only one class of
   non-genuine objects; ``color pseudo-objects'', which are handled
   separately.  The closepl() routine will write out the color
   pseudo-objects first, as xfig demands, and then write out the stored
   genuine objects. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

char *_libplot_suffix = "fig";

/* meaningless for this device, but some apps assume these variables exist */
int _libplot_output_high_byte_first = 0;
int _libplot_output_is_ascii = 0;

int _libplot_have_font_metrics = 1;	
int _libplot_have_vector_fonts = 1;
int _libplot_have_ps_fonts = 1;
int _libplot_have_wide_lines = 1;

/* our default output buffer */
Outbuffer _outbuf;

/* whether or not we're inside an openpl()...closepl() pair */
Boolean _grdevice_open = FALSE;

int
openpl ()
{
  if (_grdevice_open)
    {
      fprintf (stderr, "libplot: openpl() called when graphics device already open\n");
      return -1;
    }
  else
    _grdevice_open = TRUE;

  /* prepare the buffer in which we'll store all [genuine] objects */
  _initialize_buffer (&_outbuf);

  /* create drawing state, add it as the first member of the linked list */
  savestate();			

  return 0;
}

