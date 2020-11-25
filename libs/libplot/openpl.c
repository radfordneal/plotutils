/* This file contains the openpl routine, which is a standard part of
   libplot.  It opens the graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

char *_libplot_suffix = "";

int _libplot_output_is_ascii = 0; /* by default, output GNU plot(5) format */
int _libplot_output_high_byte_first = 0; /* by default, use system format */
int _libplot_have_font_metrics = 0; /* in raw libplot, no info available */
int _libplot_have_vector_fonts = 0; /* in raw libplot, no info available */
int _libplot_have_ps_fonts = 0;	/* in raw libplot, no info available */
int _libplot_have_wide_lines = 2; /* no guarantee; depends on translator */

/* meaningless for this version, but some apps assume it exists */
int _libplot_max_unfilled_polyline_length = 0;

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

  return 0;
}
