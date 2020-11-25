/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* external functions in api.c, at least for the C binding */
extern void _close_other_plotter_fds __P((Plotter *plotter));
extern void _flush_plotter_outstreams __P ((void));

int
#ifdef _HAVE_PROTOS
_x_closepl (void)
#else
_x_closepl ()
#endif
{
  pid_t forkval;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  _handle_x_events();

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate();
    }
  
  /* flush output streams for all Plotters before forking */
  _flush_plotter_outstreams();
  
  forkval = fork();
  if ((int)forkval > 0)		/* parent */
    {
      /* Close connection to X display associated with the window that the
	 child process will manage, i.e. with the last openpl() invoked on
	 this plotter. */
      if (close (ConnectionNumber (_plotter->dpy)) < 0)
	{
	  _plotter->error ("couldn't close connection to X display");
	  exit (1);
	}

      /* save pid of child */
      if (_plotter->num_pids == 0)
	_plotter->pids = (pid_t *)_plot_xmalloc (sizeof (pid_t));
      else
	_plotter->pids = 
	  (pid_t *)_plot_xrealloc (_plotter->pids,
				(unsigned int)((_plotter->num_pids + 1)
					       * sizeof (pid_t)));
      _plotter->pids[_plotter->num_pids] = forkval;
      _plotter->num_pids++;

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
  else if ((int)forkval == 0)	/* child */
    {
      /* close all connections to X display other than the one associated
	 with the window that we (the child) will manage */
      _close_other_plotter_fds (_plotter);

      /* In effect, send an expose event to ourselves: copy the Label
	 widget's background pixmap into its window.  (We've been drawing
	 into the two of them simultaneously, but the window could have
	 been damaged at some point in the openpl..closepl, or the X server
	 could have refused to provide backing store.)  By convention (see
	 x_openpl.c), drawable1 is the pixmap and drawable2 the window. */
      XCopyArea (_plotter->dpy, 
		 _plotter->drawable1, _plotter->drawable2,
		 _plotter->drawstate->gc, 
		 0, 0, 
		 (unsigned int)((_plotter->imax - _plotter->imin) + 1),
		 /* flipped y */
		 (unsigned int)((_plotter->jmin - _plotter->jmax) + 1),
		 0, 0);

      _plotter->open = false;	/* flag Plotter as closed */

      /* Manage the window.  We won't get any events associated with other
	 windows i.e. with previous invocations of openpl..closepl on this
	 Plotter, or with other Plotters, since we have our own application
	 context. */
      XtAppMainLoop (_plotter->app_con); /* shouldn't return */

      /* NOTREACHED */
      exit (2);
    }
  else				/* fork failed */
    {
      _plotter->error ("couldn't fork process");
      return -1;
    }
}
