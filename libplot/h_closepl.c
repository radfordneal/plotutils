/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   This version is for both HPGLPlotters and PCLPlotters.  

   For HPGL Plotter objects, we output all plotted objects, which we have
   saved in a resizable outbuf structure for the current page.  An HP-GL or
   HP-GL/2 prologue and epilogue are included.  We then fflush the
   _plotter->outstream and reset all datastructures.

   For PCL Plotters, we don't actually emit anything; we're maintaining a
   linked list of pages, and output is accomplished when the Plotters is
   deleted (see h_defplot.c). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_closepl (void)
#else
_h_closepl ()
#endif
{
  int retval = 0;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate();
    }
  
  /* output HP-GL epilogue */

  if (_plotter->pendown == true)
    /* lift pen */
    {
      sprintf (_plotter->page->point, "PU;");
      _update_buffer (_plotter->page);
    }
  /* move to lower left hand corner */
  sprintf (_plotter->page->point, "PA0,0;");
  _update_buffer (_plotter->page);

  /* select pen zero, i.e. return pen to carousel */
  if (_plotter->pen != 0)
    {
      sprintf (_plotter->page->point, "SP0;");
      _update_buffer (_plotter->page);
    }

  if (_plotter->hpgl_version >= 1)
    /* have a `page advance' command, so use it */
    {
      sprintf (_plotter->page->point, "PG0;");
      _update_buffer (_plotter->page);
    }

  /* add newline at end */
  sprintf (_plotter->page->point, "\n");
  _update_buffer (_plotter->page);

  if (_plotter->type == PL_PCL)
    /* switch back from HP-GL/2 to PCL 5 mode */
    {
      strcpy (_plotter->page->point, "\033%0A");
      _update_buffer (_plotter->page);
    }

  /* set this, so that no drawing on the next page will take place without
     a pen advance */
  _plotter->position_is_unknown = true;

  if (_plotter->outstream)
    {
      /* OUTPUT HP-GL or HP-GL/2 FOR THIS PAGE */
      if (_plotter->page->len > 0)
	/* should always be true, since we just wrote some */
	fputs (_plotter->page->base, _plotter->outstream); 
      if (fflush(_plotter->outstream) < 0)
	{
	  _plotter->error ("output stream jammed");
	  retval = -1;
	}
    }
  /* Delete the page buffer, since HP-GL Plotters don't maintain a
     linked list of pages. */
  _delete_outbuf (_plotter->page);
  _plotter->page = NULL;
  
  /* remove the zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);

  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  _plotter->pendown = false;
  _plotter->open = false;	/* flag device as closed */

  return retval;
}
