/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This generic version is for Plotters that do not plot in real time, but
   emit a page of graphics when closepl() is called. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_openpl (void)
#else
_g_openpl ()
#endif
{
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* Prepare buffer in which we'll cache graphics code for this page.
     Although we won't be emitting graphics in real time, we don't maintain
     a linked list of buffers, one per page; we don't need to. */
  _plotter->page = _new_outbuf ();
  
  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}

/* This version is used for Plotters that emit graphics only after all
   pages of graphics have been drawn, and the Plotter is deleted.  E.g.,
   for PS Plotters; this delayed output allows the output to satisfy the
   DSC (Document Structuring Conventions). */

int
#ifdef _HAVE_PROTOS
_g_openpl2 (void)
#else
_g_openpl2 ()
#endif
{
  Outbuf *new_page;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* prepare buffer in which we'll cache code for this page */
  new_page = _new_outbuf ();

  if (_plotter->opened == false) /* first page */
    {
      _plotter->page = new_page;
      /* Save a pointer to the first page, since we'll be caching
	 all pages until the Plotter is deleted. */
      _plotter->first_page = new_page;
    }
  else
    /* add new page to tail of list, update pointer to current page */
    {
      _plotter->page->next = new_page;
      _plotter->page = new_page;
    }

  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}

