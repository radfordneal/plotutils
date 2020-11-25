/* This file contains the openpl routine, which is a standard part of
   libplot.  It opens the graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

char *_libplot_suffix = "tek";

/* meaningless for this device, but some apps assume these variables exist */
int _libplot_output_high_byte_first = 0;
int _libplot_output_is_ascii = 0;

int _libplot_have_font_metrics = 1;	
int _libplot_have_vector_fonts = 1;
int _libplot_have_ps_fonts = 0;
int _libplot_have_wide_lines = 0;

/* whether or not we're inside an openpl()...closepl() pair */
Boolean _grdevice_open = FALSE;

/* first three initializations here are irrelevant because of the three
   following TRUE's: they flag all three elements (Tek mode, Tek line type,
   and Tek's graphics cursor position) as unknown at startup time */

TekState _tekstate = 
{
  MODE_ALPHA,			
  L_SOLID,
  {0, 0},
  TRUE,
  TRUE,
  TRUE
};

display_type _tek_display_type = D_GENERIC;

int
openpl ()
{
  char* term_type;

  if (_grdevice_open)
    {
      fprintf (stderr, "libplot: openpl() called when graphics device already open\n");
      return -1;
    }
  else
    _grdevice_open = TRUE;

  /* following tests are not completely reliable; stdout could be redirected */
  term_type = getenv ("TERM");
  if (term_type != NULL)
    {
      if ((strcmp (term_type, "xterm") == 0)
	  || (strcmp (term_type, "xterms") == 0)
	  || (strcmp (term_type, "kterm") == 0))
	_tek_display_type = D_XTERM;
      else if ((strcmp (term_type, "ansi.sys") == 0)
	       || (strcmp (term_type, "ansi.sysk") == 0)
	       || (strcmp (term_type, "ansisys") == 0)
	       || (strcmp (term_type, "ansisysk") == 0)
	       || (strcmp (term_type, "kermit") == 0))
	_tek_display_type = D_KERMIT;
      else
	_tek_display_type = D_GENERIC;
    }
  
  if (_tek_display_type == D_XTERM || _tek_display_type == D_KERMIT)
      /* VT340 command, put xterm / kermit into Tek mode.*/
      fprintf (_outstream, "\033[?38h");

  if (_tek_display_type == D_KERMIT)
    {
      /* select default background color [presumably black];
	 do we need this? */
      fprintf (_outstream,
	       _kermit_bgcolor_escapes[_default_drawstate.kermit_bgcolor]);
      /* select default foreground color [presumably white] */
      fprintf (_outstream, 
	       _kermit_fgcolor_escapes[_default_drawstate.kermit_fgcolor]);
    }

  /* create drawing state, add it as the first member of the linked list */
  savestate();			

  return 0;
}
