/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
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
_x_erase (void)
#else
_x_erase ()
#endif
{
  bool head_found;
  int window_width, window_height;
  int i, current_frame;
  Colorrecord *cptr, **link = NULL;
  State *stateptr;

  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* set the foreground color in the GC we use for erasing,
     to be the user-specified background color */
  _plotter->set_bg_color ();

  /* compute rectangle size; note flipped-y convention */
  window_width = (_plotter->imax - _plotter->imin) + 1;
  window_height = (_plotter->jmin - _plotter->jmax) + 1;

  if (_plotter->double_buffering)
    {
      /* copy current frame of buffered graphics to drawable(s) */
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
      /* erase graphics buffer by filling with background color */
      XFillRectangle (_plotter->dpy, _plotter->drawable3, 
		      _plotter->drawstate->gc_bg,
		      /* upper left corner */
		      0, 0,
		      (unsigned int)window_width, (unsigned int)window_height);
    }
  else
    /* not double buffering */
    {
      /* erase drawable(s) by filling with background color */
      if (_plotter->drawable1)
	XFillRectangle (_plotter->dpy, _plotter->drawable1, 
			_plotter->drawstate->gc_bg,
			/* upper left corner */
			0, 0,
			(unsigned int)window_width, (unsigned int)window_height);
      if (_plotter->drawable2)
	XFillRectangle (_plotter->dpy, _plotter->drawable2, 
			_plotter->drawstate->gc_bg,
			/* upper left corner */
			0, 0,
			(unsigned int)window_width, (unsigned int)window_height);
    }
  
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
   */	   
  cptr = _plotter->x_colorlist;
  _plotter->x_colorlist = NULL;
  i = 0;
  head_found = false;
  current_frame = _plotter->frame_number;
  while (cptr)
    {
      Colorrecord *cptrnext;

      cptrnext = cptr->next;
      if (cptr->allocated)
	{
	  if ((_plotter->double_buffering == false
	      && i < NUM_KEPT_COLORS)
	      ||
	      (_plotter->double_buffering == true
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
	      XFreeColors (_plotter->dpy, _plotter->cmap, 
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
      stateptr->x_fgcolor_status = false;
      stateptr->x_fillcolor_status = false;
      stateptr->x_bgcolor_status = false;
    }

  /* onward, to the next frame */
  _plotter->frame_number++;

  _handle_x_events();
  return 0;
}
