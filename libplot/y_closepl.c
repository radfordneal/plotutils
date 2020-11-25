/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. 

   This implementation is for XDrawablePlotters. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_y_closepl (void)
#else
_y_closepl ()
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
  
  /* elements of state that are strings etc. are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  /* free graphics context, if we have one -- and to have one (see
     x_savestate.c), must have at least one drawable */
  if (_plotter->drawable1 || _plotter->drawable2)
    XFreeGC (_plotter->dpy, _plotter->drawstate->gc);
  
  free (_plotter->drawstate);
  _plotter->drawstate = (State *)NULL;
  
  _plotter->open = false;	/* flag Plotter as closed */
  
  return 0;
}
