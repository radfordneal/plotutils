/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   This version is for both HPGLPlotters and PCLPlotters.  

   For HPGL Plotter objects, we output all plotted objects, which we have
   saved in a resizable outbuf structure for the current page.  An HP-GL or
   HP-GL/2 prologue and epilogue are included.  We then flush the output
   stream, and reset all datastructures. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_closepl (void)
#else
_h_closepl ()
#endif
{
  int retval;

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

  /* if a PCL Plotter, switch back from HP-GL/2 mode to PCL mode */
  _maybe_switch_from_hpgl ();
  
  /* set this, so that no drawing on the next page will take place without
     a pen advance */
  _plotter->hpgl_position_is_unknown = true;

  /* OUTPUT HP-GL or HP-GL/2 FOR THIS PAGE */
  if (_plotter->page->len > 0)
    /* should always be true, since we just wrote some */
    _plotter->write_string (_plotter->page->base); 

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

  /* attempt to flush (will test whether stream is jammed) */
  retval = _plotter->flushpl ();

  _plotter->pendown = false;
  _plotter->open = false;	/* flag device as closed */

  return retval;
}

void
#ifdef _HAVE_PROTOS
_h_maybe_switch_from_hpgl (void)
#else
_h_maybe_switch_from_hpgl ()
#endif
{
}

void
#ifdef _HAVE_PROTOS
_q_maybe_switch_from_hpgl (void)
#else
_q_maybe_switch_from_hpgl ()
#endif
{
  /* switch back from HP-GL/2 to PCL 5 mode */
  strcpy (_plotter->page->point, "\033%0A");
  _update_buffer (_plotter->page);
}
