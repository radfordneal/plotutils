/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

/* This version is used for PSPlotters, which emit graphics only after all
   pages of graphics have been drawn, and the Plotter is deleted.  Such
   Plotters maintain a linked list of pages (graphics are only written to
   the output stream when a Plotter is deleted, and the appropriate
   `terminate' method is invoked).  So this version simply finalizes the
   current page by invoking endpath() etc.; it doesn't write anything
   out. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_p_closepl(S___(Plotter *_plotter))
#else
_p_closepl(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate (S___(_plotter));
    }
  
  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  _plotter->open = false;	/* flag device as closed */

  return 0;
}
