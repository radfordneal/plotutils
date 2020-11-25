/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for PS Plotters.  It is just like g_openpl.c, the
   version used by Plotters that cache each page of graphics rather than
   emitting graphics in real time.  But it saves a pointer to the first
   page, since a PS Plotter emits graphics only after all pages of graphics
   have been drawn, and the Plotter is deleted.  This allows the emitted PS
   to satisfy the DSC (Document Structuring Conventions).  See
   p_terminate.c. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_p_openpl (void)
#else
_p_openpl ()
#endif
{
  Outbuf *new_page;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* prepare buffer in which we'll cache Postscript code for this page */
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

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}
