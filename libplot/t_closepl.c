/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_t_closepl (void)
#else
_t_closepl ()
#endif
{
  int retval;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate();
    }
  
  _tek_move (0, 0);		/* move to lower left corner in Tek space */
  _tek_mode (MODE_ALPHA);	/* switch to alpha mode, for user's sake */

  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  switch (_plotter->tek_display_type) /* exit from Tek mode */
    {
    case D_KERMIT:
      /* use VT340 command to exit graphics mode */
      _plotter->write_string ("\033[?38l");
      /* following command may be an alternative */
      /*
	_plotter->write_string ("\030");
      */
      break;
    case D_XTERM:
      _plotter->write_string ("\033\003"); /* ESC C-c, restore to VT102 mode */
      break;
    case D_GENERIC:
    default:
      break;
    }
  
  /* attempt to flush (will test whether stream is jammed) */
  retval = _plotter->flushpl ();

  _plotter->open = false;	/* flag device as closed */

  return retval;
}
