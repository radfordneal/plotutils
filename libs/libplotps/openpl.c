/* This file contains the openpl routine, which is a standard part of
   libplot.  It opens the graphics device.

   For libplotps, we initialize the line type and choice of font, and
   initialize the buffer in which we'll store Postscript code.  We don't
   print out the postscript prologue, since we'll know the bounding box
   only after all objects have been plotted.  The closepl() routine will
   write everything out. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

char *_libplot_suffix = "ps";

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

  /* prepare buffer in which we'll cache all Postscript code */
  _initialize_buffer (&_outbuf);

  /* reset range bounds (for BoundingBox) */
  _reset_range();

  /* create drawing state, add it as the first member of the linked list */
  savestate();			

  return 0;
}
