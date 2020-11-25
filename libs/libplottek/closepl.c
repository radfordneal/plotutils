/* This file contains the closepl routine, which is a standard part of
   libplot.  It closes the graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
closepl ()
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: closepl() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _tek_move (0, 0, TRUE);	/* move to lower left corner in Tek space */
	  			/* (caution: tek_move() may change mode) */
  _drawstate->tek_mode = MODE_ALPHA; /* set our notion of the mode */
  _tek_mode ();			/* sync Tek's notion of mode with ours */

  fflush(_outstream);

  if (_drawstate->previous != NULL)
    {
      fprintf (stderr, "libplot: closepl() called at state stack depth >1\n");
      while (_drawstate->previous)
	restorestate();		/* pop extraneous state off the stack */
    }
  
  /* remove the zeroth state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_drawstate->line_mode);
  free (_drawstate->join_mode);
  free (_drawstate->cap_mode);
  free (_drawstate->font_name);
  
  free (_drawstate);
  _drawstate = NULL;

  _grdevice_open = FALSE;	/* flag device as closed */

  switch (_tek_display_type)	/* exit from Tek mode */
    {
    case D_KERMIT:
      /* originally used this VT340 command to exit graphics mode */
      /*
      fprintf (_outstream, "\033[?38l");
      */
      /* this, instead, may be all that is needed */
      fprintf (_outstream, "\030");
      /* hope the following are what the user wants (do we need these?) */
      fprintf (_outstream,
	       _kermit_bgcolor_escapes[ANSI_SYS_BLACK]);
      fprintf (_outstream,
	       _kermit_fgcolor_escapes[ANSI_SYS_GRAY55]);
      break;
    case D_XTERM:
      fprintf (_outstream, "\033\003"); /* ESC C-c, restore to VT102 mode */  
      break;
    case D_GENERIC:
    default:
      break;
    }

  return 0;
}
