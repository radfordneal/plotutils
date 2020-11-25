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

  if (_drawstate->previous)
    {
      fprintf (stderr, "libplot: closepl() called at state stack depth >1\n");
      while (_drawstate->previous)
	restorestate();		/* pop extraneous state(s) off the stack */
    }
  
  if (fork ())		/* parent */
    {
      /* remove zeroth state too, so we can start afresh */
      
      /* elements of state that are strings etc. are freed separately */
      free (_drawstate->line_mode);
      free (_drawstate->join_mode);
      free (_drawstate->cap_mode);
      free (_drawstate->font_name);
      XFreeGC (_xdata.dpy, _drawstate->gc);
      
      free (_drawstate);
      _drawstate = (State *)NULL;
      
      _grdevice_open = FALSE;	/* flag device as closed */
      
      return close (ConnectionNumber(_xdata.dpy));
    }
  
  else			/* child */
    {
      /* in effect, send an expose event to ourself */
      XCopyArea (_xdata.dpy, _xdata.pixmap, _xdata.window, 
		 _drawstate->gc, 
		 0, 0, _xdata.width, _xdata.height, 0, 0);

      XtAppMainLoop(_xdata.app_con); /* shouldn't return */
      /* NOTREACHED */
      exit (2);
    }
}
