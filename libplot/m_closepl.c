/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_closepl (S___(Plotter *_plotter))
#else
_m_closepl (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int retval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "closepl: invalid operation");
      return -1;
    }

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	/* use generic method; we don't want to emit an op code */
	_g_restorestate (S___(_plotter));
    }

  _meta_emit_byte (R___(_plotter) (int)O_CLOSEPL);
  _meta_emit_terminator (S___(_plotter));

  /* attempt to flush (will test whether stream is jammed) */
  retval = _plotter->flushpl (S___(_plotter));

  _plotter->open = false;	/* flag device as closed */

  return retval;
}
