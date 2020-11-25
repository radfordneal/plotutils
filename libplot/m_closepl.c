/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_closepl (void)
#else
_m_closepl ()
#endif
{
  int retval;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	/* use generic method; we don't want to emit an op code */
	_g_restorestate();
    }

  _meta_emit_byte ((int)O_CLOSEPL);
  _meta_emit_terminator ();

  /* attempt to flush (will test whether stream is jammed) */
  retval = _plotter->flushpl ();

  _plotter->open = false;	/* flag device as closed */

  return retval;
}
