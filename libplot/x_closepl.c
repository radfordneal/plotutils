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
  Pixmap bg_pixmap = (Pixmap)0;
  int window_width, window_height;
  pid_t forkval;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* compute rectangle size; note flipped-y convention */
  window_width = (_plotter->imax - _plotter->imin) + 1;
  window_height = (_plotter->jmin - _plotter->jmax) + 1;

  /* if either sort of server-supported double buffering is being used,
     create background pixmap for Label widget (it doesn't yet have one) */
  if (_plotter->double_buffering == DBL_MBX
      || _plotter->double_buffering == DBL_DBE)
    {
      int screen;		/* screen number */
      Screen *screen_struct;	/* screen structure */

      screen = DefaultScreen (_plotter->dpy);
      screen_struct = ScreenOfDisplay (_plotter->dpy, screen);
      bg_pixmap = XCreatePixmap(_plotter->dpy, 
				_plotter->drawable2,
				(unsigned int)window_width, 
				(unsigned int)window_height, 
				(unsigned int)PlanesOfScreen(screen_struct));

      /* copy from off-screen graphics buffer to pixmap */
      XCopyArea (_plotter->dpy, _plotter->drawable3, bg_pixmap,
		 _plotter->drawstate->gc_bg,		   
		 0, 0,
		 (unsigned int)window_width, (unsigned int)window_height,
		 0, 0);

      /* pixmap is installed below as background pixmap for Label widget */
    }
  
  /* If double buffering, must make final frame of graphics visible, by
     copying it from our off-screen graphics buffer `drawable3' to window.
     There are several types of double buffering: the two server-supported
     types, and the `by hand' type. */

#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#ifdef HAVE_DBE_SUPPORT
  if (_plotter->double_buffering == DBL_DBE)
    /* we're using the X double buffering extension; off-screen graphics
       buffer `drawable3' is a back buffer */
    {
      XdbeSwapInfo info;
      
      /* make final frame of graphics visible by interchanging front and
         back buffers one last time */
      info.swap_window = _plotter->drawable2;
      info.swap_action = XdbeUndefined;
      XdbeSwapBuffers (_plotter->dpy, &info, 1);

      /* free the back buffer */
      XdbeDeallocateBackBufferName (_plotter->dpy, _plotter->drawable3);
    }
#endif /* HAVE_DBE_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_XDBE_H */

#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#ifdef HAVE_MBX_SUPPORT
  if (_plotter->double_buffering == DBL_MBX)
    /* we're using the X multibuffering extension; off-screen graphics
       buffer `drawable3' is a non-displayed multibuffer */
    {
      /* make final frame of graphics visible by making the multibuffer
	 into which we're currently drawing the on-screen multibuffer */
      XmbufDisplayBuffers (_plotter->dpy, 1, &(_plotter->drawable3), 0, 0);
    }
#endif /* HAVE_MBX_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_MULTIBUF_H */

  /* if either sort of server-supported double buffering is being used,
     install the above-created pixmap as background pixmap for the Label
     widget to use, once the window has been spun off */
  if (_plotter->double_buffering == DBL_MBX
      || _plotter->double_buffering == DBL_DBE)
    {
      Arg wargs[10];		/* werewolves */

      /* install pixmap as Label widget's background pixmap */
#ifdef USE_MOTIF
      XtSetArg (wargs[0], XmNlabelPixmap, (Pixmap)bg_pixmap);
      XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
      XtSetValues (_plotter->canvas, wargs, (Cardinal)2);
#else
      XtSetArg (wargs[0], XtNbitmap, (Pixmap)bg_pixmap);
      XtSetValues (_plotter->canvas, wargs, (Cardinal)1);
#endif
    }
  
  if (_plotter->double_buffering == DBL_BY_HAND)
    /* we're double buffering _manually_, rather than using either X11
       protocol extension, so our off-screen graphics buffer `drawable3' is
       an ordinary pixmap */
	{
	  /* make final frame of graphics visible by copying from pixmap to
             window */
	  XCopyArea (_plotter->dpy, _plotter->drawable3, _plotter->drawable2,
		     _plotter->drawstate->gc_bg,		   
		     0, 0,
		     (unsigned int)window_width, (unsigned int)window_height,
		     0, 0);
	}

  /* Finally: if we're not double buffering at all, we copy our off-screen
     graphics buffer to the window.  The off-screen graphics buffer is just
     the Label widget's background pixmap, `drawable1'. */
  if (_plotter->double_buffering == DBL_NONE)
    XCopyArea (_plotter->dpy, _plotter->drawable1, _plotter->drawable2,
	       _plotter->drawstate->gc_bg,		   
	       0, 0,
	       (unsigned int)window_width, (unsigned int)window_height,
	       0, 0);

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

  /* Free font records from Plotter's cache list.  This involves
     deallocating the font name and also the XFontStruct contained in each
     record, if non-NULL.  (NULL indicates that the font could not be
     retrieved.)  */
  fptr = _plotter->x_fontlist;
  _plotter->x_fontlist = NULL;
  while (fptr)
    {
      Fontrecord *fptrnext;

      fptrnext = fptr->next;
      free (fptr->name);
      if (fptr->x_font_struct)
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

  /* flush out the X output buffer; wait till all requests have been
     received and processed by server */
  _plotter->flushpl ();

  /* flush output streams for all Plotters before forking */
  _flush_plotter_outstreams();
  
  /* DO IT */
  forkval = fork();
  if ((int)forkval > 0)		/* parent */
    {
      int retval = 0;

      /* Close connection to X display associated with the window that the
	 child process will manage, i.e. with the last openpl() invoked on
	 this plotter. */
      if (close (ConnectionNumber (_plotter->dpy)) < 0)
	/* shouldn't happen */
	{
	  _plotter->error ("couldn't close connection to X display");
	  retval = -1;
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

      return retval;
    }
  else if ((int)forkval == 0)	/* child */
    {
      bool need_redisplay = false;

      /* close all connections to X display other than the one associated
	 with the window that we (the child) will manage */
      _close_other_plotter_fds (_plotter);

      /* Repaint by sending an expose event to ourselves, copying the Label
	 widget's background pixmap into its window.  This is a good idea
	 because the window could have been resized during the
	 openpl..closepl.  We don't do this if not double buffering (and
	 presumably animating), unless the window size has changed since
	 openpl was invoked (repainting makes the window flash, possibly
	 irritating users). */
      if (_plotter->double_buffering != DBL_NONE)
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
	  XtGetValues (_plotter->canvas, wargs, (Cardinal)2);
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
	XChangeWindowAttributes (_plotter->dpy, (Window)_plotter->drawable2, 
				 value_mask, &attributes);
      }

      if (need_redisplay)
	/* send expose event to ourselves */
	XClearArea (_plotter->dpy, 
		    (Window)_plotter->drawable2, 
		    0, 0, 
		    (unsigned int)0, (unsigned int)0, 
		    True);
      
      _plotter->open = false;	/* flag Plotter as closed */
      
      /* Manage the window.  We won't get any events associated with other
	 windows i.e. with previous invocations of openpl..closepl on this
	 Plotter, or with other Plotters, since we have our own application
	 context. */
      XtAppMainLoop (_plotter->app_con); /* shouldn't return */

      /* NOTREACHED */
      exit (1);
    }
  else				/* fork failed */
    {
      _plotter->error ("couldn't fork process");
      return -1;
    }
}
