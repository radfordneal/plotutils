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

  if (_plotter->double_buffering)
    /* copy final frame of buffered graphics from pixmap serving as
       graphics buffer, to window */
    {
      /* compute rectangle size; note flipped-y convention */
      int window_width = (_plotter->imax - _plotter->imin) + 1;
      int window_height = (_plotter->jmin - _plotter->jmax) + 1;
      
      if (_plotter->drawable1)
	XCopyArea (_plotter->dpy, _plotter->drawable3, _plotter->drawable1,
		   _plotter->drawstate->gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      if (_plotter->drawable2)
	XCopyArea (_plotter->dpy, _plotter->drawable3, _plotter->drawable2,
		   _plotter->drawstate->gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
    }

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
  /* free graphics contexts, if we have them -- and to have them, must have
     at least one drawable (see x_savestate.c) */
  if (_plotter->drawable1 || _plotter->drawable2)
    {
      XFreeGC (_plotter->dpy, _plotter->drawstate->gc_fg);
      XFreeGC (_plotter->dpy, _plotter->drawstate->gc_fill);
      XFreeGC (_plotter->dpy, _plotter->drawstate->gc_bg);
    }
  
  free (_plotter->drawstate);
  _plotter->drawstate = (State *)NULL;
  
  _plotter->open = false;	/* flag Plotter as closed */
  
  return 0;
}
