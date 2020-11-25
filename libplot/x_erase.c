/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

/* This version is for XDrawablePlotters, and should be identical to
   y_erase.c (the version for XPlotters) except that XPlotter-specific
   lines are commented out.  (Search for #if 0...#endif.)  The function
   y_erase() is of course renamed x_erase() here. */

#include "sys-defines.h"
#include "extern.h"

/* If we aren't double buffering, this is the number of
   most-recently-allocated color cells that we _don't_ deallocate when we
   do an erase().  This is a heuristic.  This quantity must be >= 0. */
#define NUM_KEPT_COLORS 256

/* If we're doing double buffering, when we do an erase() we of course
   don't deallocate the color cells that were used in the current frame.
   We also don't deallocate the cells used in the previous NUM_KEPT_FRAMES
   frames.  This is a heuristic.  This quantity must be >= 0. */
#define NUM_KEPT_FRAMES 16

int
#ifdef _HAVE_PROTOS
_x_erase (S___(Plotter *_plotter))
#else
_x_erase (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool head_found;
  int window_width, window_height;
  int i, current_frame;
  plColorRecord *cptr, **link = NULL;
  plDrawState *stateptr;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "erase: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* set the foreground color in the GC we use for erasing,
     to be the user-specified background color */
  _plotter->set_bg_color (S___(_plotter));

  /* compute rectangle size; note flipped-y convention */
  window_width = (_plotter->imax - _plotter->imin) + 1;
  window_height = (_plotter->jmin - _plotter->jmax) + 1;

  if (_plotter->x_double_buffering != DBL_NONE)
    {
      /* Following two sorts of server-supported double buffering
	 (DBL_DBE, DBL_MBX) are possible only for X Plotters, not
	 X Drawable Plotters.  `By hand' double buffering is possible
	 for both. */

#if 0
#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#ifdef HAVE_DBE_SUPPORT
      if (_plotter->x_double_buffering == DBL_DBE)
	/* we're using the X double buffering extension */
	{
	  XdbeSwapInfo info;
	  
	  /* Copy current frame of buffered graphics to window.  Implement
	     this by swapping the front and back buffers for widget's
	     window.  Former front buffer will become graphics buffer.
	     Currently, the buffers are `x_drawable2' (front) and `x_drawable3'
	     (back, into which we draw). */
	  info.swap_window = _plotter->x_drawable2;
	  info.swap_action = XdbeUndefined;
	  XdbeSwapBuffers (_plotter->x_dpy, &info, 1);
	}
      else
#endif /* HAVE_DBE_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_XDBE_H */

#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#ifdef HAVE_MBX_SUPPORT
      if (_plotter->x_double_buffering == DBL_MBX)
	/* we're using the X multibuffering extension */
	{
	  Multibuffer multibuf;

	  /* Copy current frame of buffered graphics to window.  Implement
	     this by making multibuffer into which we've been drawing the
	     current multibuffer. */
	  XmbufDisplayBuffers (_plotter->x_dpy, 1, &(_plotter->x_drawable3), 0, 0);

	  /* swap the two multibuffers, making the other one the off-screen
	     graphics buffer into which we draw (`x_drawable3') */
	  multibuf = _plotter->x_drawable3;
	  _plotter->x_drawable3 = _plotter->y_drawable4;
	  _plotter->y_drawable4 = multibuf;
	}
      else
#endif /* HAVE_MBX_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_MULTIBUF_H */
#endif /* 0 */

      /* we must be doing double buffering `by hand', rather than using
         an X protocol extension */
      if (_plotter->x_double_buffering == DBL_BY_HAND)
	{
	  /* copy current frame of buffered graphics to drawable(s) */
	  if (_plotter->x_drawable1)
	    XCopyArea (_plotter->x_dpy, 
		       _plotter->x_drawable3, _plotter->x_drawable1,
		       _plotter->drawstate->x_gc_bg,		   
		       0, 0,
		       (unsigned int)window_width, 
		       (unsigned int)window_height,
		       0, 0);
	  if (_plotter->x_drawable2)
	    XCopyArea (_plotter->x_dpy, 
		       _plotter->x_drawable3, _plotter->x_drawable2,
		       _plotter->drawstate->x_gc_bg,		   
		       0, 0,
		       (unsigned int)window_width, 
		       (unsigned int)window_height,
		       0, 0);
	}

      /* irrespective of which of the three sorts of double buffering is
	 being performed, clear the (new) graphics buffer, by filling it
	 with background color */
      XFillRectangle (_plotter->x_dpy, _plotter->x_drawable3, 
		      _plotter->drawstate->x_gc_bg,
		      /* upper left corner */
		      0, 0,
		      (unsigned int)window_width, 
		      (unsigned int)window_height);
    }
  else
    /* not double buffering at all */
    {
      /* erase drawable(s) by filling with background color */
      if (_plotter->x_drawable1)
	XFillRectangle (_plotter->x_dpy, _plotter->x_drawable1, 
			_plotter->drawstate->x_gc_bg,
			/* upper left corner */
			0, 0,
			(unsigned int)window_width, (unsigned int)window_height);
      if (_plotter->x_drawable2)
	XFillRectangle (_plotter->x_dpy, _plotter->x_drawable2, 
			_plotter->drawstate->x_gc_bg,
			/* upper left corner */
			0, 0,
			(unsigned int)window_width, (unsigned int)window_height);
    }

#if 0
  /* If an X Plotter, update background color of canvas widget,
     irrespective of whether or not we're double buffering.  This fixes
     things so that if the window is resized to a larger size, the new
     portions of the window will be filled with the correct color. */
  {
    Arg wargs[10];		/* werewolves */

#ifdef USE_MOTIF
    XtSetArg (wargs[0], XmNbackground, _plotter->drawstate->x_gc_bgcolor);
#else
    XtSetArg (wargs[0], XtNbackground, _plotter->drawstate->x_gc_bgcolor);
#endif
    XtSetValues (_plotter->y_toplevel, wargs, (Cardinal)1);
    XtSetValues (_plotter->canvas, wargs, (Cardinal)1);
  }
#endif /* 0 */

  /* Flush the color cell cache, to the extent we can.  But heuristically,
     keep in the cache a certain number of cells that aren't strictly
     needed, but which may be needed in the following frames.  There are
     two cases.

     1. If we're not double buffering, preserve some maximum number
          (NUM_KEPT_COLORS) of the most recently allocated cells.
          Implementing the cache as a list, though suboptimal from the
          point of view of speed, makes it easy to implement this heuristic.
     2. If we're double buffering, preserve all cells that were used
          in the present frame (which was just transferred to the
          drawable(s), e.g., to an on-screen window).  This is mandatory.
          But also use a heuristic: preserve all cells used in the 
	  preceding NUM_KEPT_FRAMES frames.

     In both cases, if a cached cell is to be preserved, it must contain a
     genuine pixel value (the `allocated' flag must be set).

     Side comment: for XDrawable Plotters, unlike XPlotters, it makes no
     sense to reset the frame number to zero at the conclusion of each
     openpl..closepl, since we've only got one connection to the X display.
     So we don't (see x_openpl.c).
   */	   
  cptr = _plotter->x_colorlist;
  _plotter->x_colorlist = NULL;
  i = 0;
  head_found = false;
  current_frame = _plotter->frame_number;
  while (cptr)
    {
      plColorRecord *cptrnext;

      cptrnext = cptr->next;
      if (cptr->allocated)
	{
	  if (((_plotter->x_double_buffering == DBL_NONE)
	      && i < NUM_KEPT_COLORS)
	      ||
	      ((_plotter->x_double_buffering != DBL_NONE)
	       && cptr->frame >= current_frame - NUM_KEPT_FRAMES))
	    {
	      if (head_found)
		*link = cptr;
	      else
		{
		  _plotter->x_colorlist = cptr;
		  head_found = true;
		}

	      cptr->next = NULL;
	      link = &(cptr->next);
	      i++;
	    }
	  else
	    /* cached cell contains a genuine pixel value, but it doesn't
	       meet our criteria, so deallocate it */
	    {
	      XFreeColors (_plotter->x_dpy, _plotter->x_cmap, 
			   &(cptr->rgb.pixel), 1, (unsigned long)0);
	      free (cptr); 
	    }
	}
      else
	/* cached cell doesn't include a genuine pixel value, so free it */
	free (cptr); 

      cptr = cptrnext;
    }

  /* flag status of all colors in GC's in the drawing state stack as false
     (on account of flushing, may need to be searched for or reallocated) */
  for (stateptr = _plotter->drawstate; stateptr; stateptr = stateptr->previous)
    {
      stateptr->x_gc_fgcolor_status = false;
      stateptr->x_gc_fillcolor_status = false;
      stateptr->x_gc_bgcolor_status = false;
    }

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  /* on to next frame */
  _plotter->frame_number++;

  return 0;
}
