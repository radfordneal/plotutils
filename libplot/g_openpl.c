/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for Plotters that do not plot in real time, but
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

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}
