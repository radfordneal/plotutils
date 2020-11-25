/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   This implementation is for XDrawablePlotters. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_closepl (S___(Plotter *_plotter))
#else
_x_closepl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* Xdrawable Plotters support double buffering `by hand', so check for it */

  if (_plotter->x_double_buffering == DBL_BY_HAND)
    /* copy final frame of buffered graphics from pixmap serving as
       graphics buffer, to window */
    {
      /* compute rectangle size; note flipped-y convention */
      int window_width = (_plotter->imax - _plotter->imin) + 1;
      int window_height = (_plotter->jmin - _plotter->jmax) + 1;
      
      if (_plotter->x_drawable1)
	XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable1,
		   _plotter->drawstate->x_gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      if (_plotter->x_drawable2)
	XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable2,
		   _plotter->drawstate->x_gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      
      /* no more need for pixmap, so free it (if there is one) */
      if (_plotter->x_drawable1 || _plotter->x_drawable2)
	XFreePixmap (_plotter->x_dpy, _plotter->x_drawable3);
    }

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate (S___(_plotter));
    }
  
  /* remove zeroth drawing state too, so we can start afresh */
  
  /* elements of state that are strings etc. are freed separately */
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->font_name);
  /* free graphics contexts, if we have them -- and to have them, must have
     at least one drawable (see x_savestate.c) */
  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_bg);
    }
  
  free (_plotter->drawstate);
  _plotter->drawstate = (plDrawState *)NULL;
  
  _plotter->open = false;	/* flag Plotter as closed */
  
  return 0;
}
