/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_closepl (void)
#else
_x_closepl ()
#endif
{
  Colorrecord *cptr;
  Fontrecord *fptr;
  pid_t forkval;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  if (_plotter->double_buffering)
    /* copy final frame of buffered graphics from pixmap to window */
    {
      /* compute rectangle size; note flipped-y convention */
      int window_width = (_plotter->imax - _plotter->imin) + 1;
      int window_height = (_plotter->jmin - _plotter->jmax) + 1;
      
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
  
  /* following two deallocations (of font records and color cell records)
     arrange things so that when drawing the next page of graphics, which
     will require another connection to the X server, the Plotter will
     start with a clean slate */

  /* Free cached font records from Plotter's cache list.  Also deallocate
     the font structure contained in each record. */
  fptr = _plotter->x_fontlist;
  _plotter->x_fontlist = NULL;
  while (fptr)
    {
      Fontrecord *fptrnext;

      fptrnext = fptr->next;
      XFreeFont (_plotter->dpy, fptr->x_font_struct);
      free (fptr); 
      fptr = fptrnext;
    }

  /* Free cached color cells from Plotter's cache list.  Do _not_ ask the
     server to deallocate the cells themselves; just free local storage. */
  cptr = _plotter->x_colorlist;
  _plotter->x_colorlist = NULL;
  while (cptr)
    {
      Colorrecord *cptrnext;

      cptrnext = cptr->next;
      free (cptr); 
      cptr = cptrnext;
    }

  /* handle any final X events */
  _handle_x_events();

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
  else if ((int)forkval == 0)	/* child */
    {
      Drawable pixmap;

      /* close all connections to X display other than the one associated
	 with the window that we (the child) will manage */
      _close_other_plotter_fds (_plotter);

      /* In effect, send an expose event to ourselves: copy the Label
	 widget's background pixmap into its window.  (We've been drawing
	 into the two of them simultaneously, but the window could have
	 been damaged at some point in the openpl..closepl, or the X server
	 could have refused to provide backing store.)  By convention (see
	 x_openpl.c), drawable1 is the pixmap and drawable2 the window.
	 Unless we've been double buffering, in which case there is no
	 drawable1, and drawable3 is the pixmap. */

      pixmap = _plotter->double_buffering ?
	_plotter->drawable3 : _plotter->drawable1;
      XCopyArea (_plotter->dpy, 
		 pixmap, _plotter->drawable2,
		 _plotter->drawstate->gc_fg, 
		 0, 0, 
		 (unsigned int)((_plotter->imax - _plotter->imin) + 1),
		 /* flipped y convention */
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
