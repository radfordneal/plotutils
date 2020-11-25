/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_closepl (void)
#else
_m_closepl ()
#endif
{
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
  _plotter->open = false;

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c\n",
		 CLOSEPL);
      else
	putc (CLOSEPL, _plotter->outstream);
    }
  
  if (_plotter->outstream && fflush (_plotter->outstream) < 0)
    {
      _plotter->error ("output stream jammed");
      return -1;
    }
  else
    return 0;
}
