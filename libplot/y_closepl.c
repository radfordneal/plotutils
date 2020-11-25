/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

/* This version is for XPlotters. */

#ifdef HAVE_WAITPID
#ifdef HAVE_SYS_WAIT_H
#define _POSIX_SOURCE		/* for waitpid() */
#endif
#endif

#include "sys-defines.h"
#include "extern.h"

#ifdef HAVE_UNISTD_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* always include before unistd.h */
#endif
#include <unistd.h>		/* for fork() */
#endif

/* song and dance to declare waitpid() and define WNOHANG */
#ifdef HAVE_WAITPID
#ifdef HAVE_SYS_WAIT_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#include <sys/wait.h>		/* for waitpid() */
#endif /* HAVE_SYS_WAIT_H */
#endif /* HAVE_WAITPID */

int
#ifdef _HAVE_PROTOS
_y_closepl (S___(Plotter *_plotter))
#else
_y_closepl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  plColorRecord *cptr;
  plFontRecord *fptr;
  Pixmap bg_pixmap = (Pixmap)0;
  int window_width, window_height;
  pid_t forkval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* compute rectangle size; note flipped-y convention */
  window_width = (_plotter->imax - _plotter->imin) + 1;
  window_height = (_plotter->jmin - _plotter->jmax) + 1;

  /* if either sort of server-supported double buffering is being used,
     create background pixmap for Label widget (it doesn't yet have one) */
  if (_plotter->x_double_buffering == DBL_MBX
      || _plotter->x_double_buffering == DBL_DBE)
    {
      int screen;		/* screen number */
      Screen *screen_struct;	/* screen structure */

      screen = DefaultScreen (_plotter->x_dpy);
      screen_struct = ScreenOfDisplay (_plotter->x_dpy, screen);
      bg_pixmap = XCreatePixmap(_plotter->x_dpy, 
				_plotter->x_drawable2,
				(unsigned int)window_width, 
				(unsigned int)window_height, 
				(unsigned int)PlanesOfScreen(screen_struct));

      /* copy from off-screen graphics buffer to pixmap */
      XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, bg_pixmap,
		 _plotter->drawstate->x_gc_bg,		   
		 0, 0,
		 (unsigned int)window_width, (unsigned int)window_height,
		 0, 0);

      /* pixmap is installed below as background pixmap for Label widget */
    }
  
  /* If double buffering, must make final frame of graphics visible, by
     copying it from our off-screen graphics buffer `x_drawable3' to window.
     There are several types of double buffering: the two server-supported
     types, and the `by hand' type. */

#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#ifdef HAVE_DBE_SUPPORT
  if (_plotter->x_double_buffering == DBL_DBE)
    /* we're using the X double buffering extension; off-screen graphics
       buffer `x_drawable3' is a back buffer */
    {
      XdbeSwapInfo info;
      
      /* make final frame of graphics visible by interchanging front and
         back buffers one last time */
      info.swap_window = _plotter->x_drawable2;
      info.swap_action = XdbeUndefined;
      XdbeSwapBuffers (_plotter->x_dpy, &info, 1);

      /* free the back buffer */
      XdbeDeallocateBackBufferName (_plotter->x_dpy, _plotter->x_drawable3);
    }
#endif /* HAVE_DBE_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_XDBE_H */

#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#ifdef HAVE_MBX_SUPPORT
  if (_plotter->x_double_buffering == DBL_MBX)
    /* we're using the X multibuffering extension; off-screen graphics
       buffer `x_drawable3' is a non-displayed multibuffer */
    {
      /* make final frame of graphics visible by making the multibuffer
	 into which we're currently drawing the on-screen multibuffer */
      XmbufDisplayBuffers (_plotter->x_dpy, 1, &(_plotter->x_drawable3), 0, 0);
    }
#endif /* HAVE_MBX_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_MULTIBUF_H */

  /* if either sort of server-supported double buffering is being used,
     install the above-created pixmap as background pixmap for the Label
     widget to use, once the window has been spun off */
  if (_plotter->x_double_buffering == DBL_MBX
      || _plotter->x_double_buffering == DBL_DBE)
    {
      Arg wargs[2];		/* werewolves */

      /* install pixmap as Label widget's background pixmap */
#ifdef USE_MOTIF
      XtSetArg (wargs[0], XmNlabelPixmap, (Pixmap)bg_pixmap);
      XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
      XtSetValues (_plotter->y_canvas, wargs, (Cardinal)2);
#else
      XtSetArg (wargs[0], XtNbitmap, (Pixmap)bg_pixmap);
      XtSetValues (_plotter->y_canvas, wargs, (Cardinal)1);
#endif
    }
  
  if (_plotter->x_double_buffering == DBL_BY_HAND)
    /* we're double buffering _manually_, rather than using either X11
       protocol extension, so our off-screen graphics buffer `x_drawable3' is
       an ordinary pixmap */
	{
	  /* make final frame of graphics visible by copying from pixmap to
             window */
	  XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable2,
		     _plotter->drawstate->x_gc_bg,		   
		     0, 0,
		     (unsigned int)window_width, (unsigned int)window_height,
		     0, 0);
	}

  /* Finally: if we're not double buffering at all, we copy our off-screen
     graphics buffer to the window.  The off-screen graphics buffer is just
     the Label widget's background pixmap, `x_drawable1'. */
  if (_plotter->x_double_buffering == DBL_NONE)
    XCopyArea (_plotter->x_dpy, _plotter->x_drawable1, _plotter->x_drawable2,
	       _plotter->drawstate->x_gc_bg,		   
	       0, 0,
	       (unsigned int)window_width, (unsigned int)window_height,
	       0, 0);

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate (S___(_plotter));
    }
  
  /* following two deallocations (of font records and color cell records)
     arrange things so that when drawing the next page of graphics, which
     will require another connection to the X server, the Plotter will
     start with a clean slate */

  /* Free font records from Plotter's cache list.  This involves
     deallocating the font name and also the XFontStruct contained in each
     record, if non-NULL.  (NULL indicates that the font could not be
     retrieved.)  */
  fptr = _plotter->x_fontlist;
  _plotter->x_fontlist = NULL;
  while (fptr)
    {
      plFontRecord *fptrnext;

      fptrnext = fptr->next;
      free (fptr->name);
      if (fptr->x_font_struct)
	XFreeFont (_plotter->x_dpy, fptr->x_font_struct);
      free (fptr); 
      fptr = fptrnext;
    }

  /* Free cached color cells from Plotter's cache list.  Do _not_ ask the
     server to deallocate the cells themselves, because the child process
     will need them; just free local storage. */
  cptr = _plotter->x_colorlist;
  _plotter->x_colorlist = NULL;
  while (cptr)
    {
      plColorRecord *cptrnext;

      cptrnext = cptr->next;
      free (cptr); 
      cptr = cptrnext;
    }

  /* A bit of last-minute cleanup (could be done elsewhere): call waitpid()
     to reclaim resources used by zombie child processes resulting from
     previous closepl()'s, if any.  If this isn't done, the controlling
     process of any previously popped-up window won't fully exit (e.g. when
     `q' is typed in the window): it'll remain in the process table as a
     zombie until the parent process executes. */
#ifdef HAVE_WAITPID
#ifdef HAVE_SYS_WAIT_H
#ifdef WNOHANG
  {
    int i;
    
    /* iterate over all previously forked-off children (should really keep
       track of which have exited, since once a child has exited, invoking
       waitpid() on it is pointless) */
    for (i = 0; i < _plotter->y_num_pids; i++)
      waitpid (_plotter->y_pids[i], (int *)NULL, WNOHANG);
  }
#endif
#endif
#endif

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  /* flush out the X output buffer; wait till all requests have been
     received and processed by server */
  _plotter->flushpl (S___(_plotter));

  /* flush output streams for all Plotters before forking */
  _flush_plotter_outstreams (S___(_plotter));
  
  /* DO IT */
  forkval = fork ();
  if ((int)forkval > 0		/* fork succeeded, and we're the parent */
      || (int)forkval < 0)	/* fork failed */
    {
      int retval = 0;

      if ((int)forkval < 0)
	_plotter->error (R___(_plotter) "couldn't fork process");	

      /* Close connection to X display associated with window that the
	 child process should manage, i.e. with the last openpl() invoked
	 on this Plotter. */
      if (close (ConnectionNumber (_plotter->x_dpy)) < 0
	  && errno != EINTR)
	/* emphatically shouldn't happen */
	{
	  _plotter->error (R___(_plotter) "couldn't close connection to X display");
	  retval = -1;
	}

      if ((int)forkval > 0)
	/* there's a child process, so save its pid */
	{
	  if (_plotter->y_num_pids == 0)
	    _plotter->y_pids = (pid_t *)_plot_xmalloc (sizeof (pid_t));
	  else
	    _plotter->y_pids = 
	      (pid_t *)_plot_xrealloc (_plotter->y_pids,
				       ((_plotter->y_num_pids + 1)
					* sizeof (pid_t)));
	  _plotter->y_pids[_plotter->y_num_pids] = forkval;
	  _plotter->y_num_pids++;
	}
      
      /* remove zeroth drawing state too, so we can start afresh */
      
      /* elements of state that are strings etc. are freed separately */
      free ((char *)_plotter->drawstate->line_mode);
      free ((char *)_plotter->drawstate->join_mode);
      free ((char *)_plotter->drawstate->cap_mode);
      free ((char *)_plotter->drawstate->font_name);

      /* free graphics contexts, if we have them -- and to have them, must
	 have at least one drawable (see x_savestate.c) */
      if (_plotter->x_drawable1 || _plotter->x_drawable2)
	{
	  XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg);
	  XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill);
	  XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_bg);
	}

      free (_plotter->drawstate);
      _plotter->drawstate = (plDrawState *)NULL;
      
      _plotter->open = false;	/* flag Plotter as closed */

      return retval;
    }

  else		/* forkval = 0; fork succeeded, and we're the child */
    {
      bool need_redisplay = false;
      int i;

      /* Alter canvas widget's translation table, so that exit will occur
	 when `q' is typed (or mouse is clicked).  See y_openpl.c. */
      _y_set_data_for_quitting (S___(_plotter));

      /* Close all connections to X display other than our own, i.e., close
	 all connections that other XPlotters may have been using.  No need
	 to lock the global variables _xplotters and _xplotters_len; since
	 we've forked and we're the child process, we're the only thread
	 left. :-)

	 We'll never be accessing those variables again (the only way we
	 could would be if we were to call _maybe_handle_x_events(), and we
	 aren't going to do that).  So we don't need to worry that they may
	 actually be locked.  I.e. there was no need for us to register a
	 handler to unlock them immediately after forking, by invoking
	 pthread_atfork().  Which is why we didn't do that. */

      for (i = 0; i < _xplotters_len; i++)
	if (_xplotters[i] != NULL
	    && _xplotters[i] != _plotter
	    && _xplotters[i]->opened
	    && _xplotters[i]->open
	    && close (ConnectionNumber (_xplotters[i]->x_dpy)) < 0
	    && errno != EINTR)
	  /* shouldn't happen */
	  _plotter->error (R___(_plotter)
			   "couldn't close connection to X display");

      /* Repaint by sending an expose event to ourselves, copying the Label
	 widget's background pixmap into its window.  This is a good idea
	 because the window could have been resized during the
	 openpl..closepl.  We don't do this if not double buffering (and
	 presumably animating), unless the window size has changed since
	 openpl was invoked (repainting makes the window flash, possibly
	 irritating users). */
      if (_plotter->x_double_buffering != DBL_NONE)
	need_redisplay = true;
      else
	{
	  Arg wargs[2];		/* werewolves */
	  Dimension window_height, window_width;

#ifdef USE_MOTIF
	  XtSetArg (wargs[0], XmNwidth, &window_width);
	  XtSetArg (wargs[1], XmNheight, &window_height);
#else
	  XtSetArg (wargs[0], XtNwidth, &window_width);
	  XtSetArg (wargs[1], XtNheight, &window_height);
#endif
	  XtGetValues (_plotter->y_canvas, wargs, (Cardinal)2);
	  if ((_plotter->imax + 1 != (int)window_width)
	      || (_plotter->jmin + 1 != (int)window_height))
	    /* window changed size */
	    need_redisplay = true;
	}

      /* turn off backing store (if used); when we send the expose event to
	 ourselves we want to repaint from the background pixmap, NOT from
	 the server's backing store */
      {
	XSetWindowAttributes attributes;
	unsigned long value_mask;
	
	attributes.backing_store = NotUseful;
	value_mask = CWBackingStore;
	XChangeWindowAttributes (_plotter->x_dpy, (Window)_plotter->x_drawable2, 
				 value_mask, &attributes);
      }

      if (need_redisplay)
	/* send expose event to ourselves */
	XClearArea (_plotter->x_dpy, 
		    (Window)_plotter->x_drawable2, 
		    0, 0, 
		    (unsigned int)0, (unsigned int)0, 
		    True);
      
      _plotter->open = false;	/* flag Plotter as closed */
      
      /* Manage the window.  We won't get any events associated with other
	 windows i.e. with previous invocations of openpl..closepl on this
	 Plotter, or with other Plotters, since we have our own application
	 context. */
      XtAppMainLoop (_plotter->y_app_con); /* shouldn't return */

      /* NOTREACHED */
      exit (EXIT_FAILURE);
    }
}
