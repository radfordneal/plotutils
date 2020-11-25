/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

/* This generic version does very little, since GenericPlotters don't
   emit any graphics code. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_closepl (void)
#else
_g_closepl ()
#endif
{
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
  
  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  _plotter->open = false;	/* flag device as closed */

  return 0;
}
