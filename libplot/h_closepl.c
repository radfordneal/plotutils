/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For HPGL Plotter objects, we first output the HP-GL or HP-GL/2 header.
   We then output all plotted objects, which we have saved in a resizable
   outbuf structure for the current page.  We fflush the
   _plotter->outstream and reset all datastructures. */

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
  
  if (_plotter->outstream)
    {
      /* output HP-GL prologue */
      if (_plotter->hpgl_version == 2)
	{
	  fprintf (_plotter->outstream, "BP;IN;");
	  fprintf (_plotter->outstream, "PS%d;",
		   IROUND(_plotter->plot_length));
	}
      else
	fprintf (_plotter->outstream, "IN;");
      
      /* rotate if user requested rotation */
      if (_plotter->rotation != 0)
	fprintf (_plotter->outstream, "RO%d;", _plotter->rotation);
      
      /* set scaling points P1, P2 at lower left and upper right corners of the
	 region (square) that we call our ``graphics display'' */
      fprintf (_plotter->outstream, "IP%d,%d,%d,%d;",
	       IROUND(_plotter->p1x), IROUND(_plotter->p1y),
	       IROUND(_plotter->p2x), IROUND(_plotter->p2y));
      
      /* Set up `scaled device coordinates' within the graphics display.  All
	 coordinates in our output file will be scaled device coordinates,
	 not physical device coordinates. */
      fprintf (_plotter->outstream, "SC%d,%d,%d,%d;",
	       IROUND (_plotter->display_coors.left), 
	       IROUND (_plotter->display_coors.right), 
	       IROUND (_plotter->display_coors.bottom), 
	       IROUND (_plotter->display_coors.top));
      
      if (_plotter->hpgl_version == 2)
	{
	  /* Begin to define a palette, by specifying a number of logical pens.
	     (All HP-GL/2 devices should support the `NP' instruction, even
	     though many support only a default palette.) */
	  if (_plotter->palette)
	    fprintf (_plotter->outstream, "NP%d;", MAX_NUM_PENS);
	  /* use relative units for pen width */
	  fprintf (_plotter->outstream, "WU1;");
	}
      
      /* select pen #1 (standard plotting convention) */
      fprintf (_plotter->outstream, "SP1;");
      
      /* For HP-GL/2 devices, set transparency mode to `opaque', if the user
	 allows it.  It should always be opaque to agree with libplot
	 conventions, but on some HP-GL/2 devices (mostly pen plotters) the
	 `TR' command does not NOP gracefully. */
      if (_plotter->hpgl_version == 2 && _plotter->opaque_mode)
	fprintf (_plotter->outstream, "TR0;");
      
      /* output cached HP-GL or HP-GL/2 */
      if (_plotter->page->len > 0)
	fputs (_plotter->page->base, _plotter->outstream); 
      
      /* output HP-GL epilogue */
      if (_plotter->pendown == true)
	/* lift pen */
	fprintf (_plotter->outstream, "PU;");
      fprintf (_plotter->outstream, "PA0,0;");
      if (_plotter->pen != 0)
	/* select pen zero, i.e. return pen to carousel */
	fprintf (_plotter->outstream, "SP0;");
      if (_plotter->hpgl_version >= 1)
	/* have a `page advance' command, so use it */
	fprintf (_plotter->outstream, "PG0;");

      fprintf (_plotter->outstream, "\n");
    }
  
  /* set this, so that no drawing on the next page will take place without
     a pen advance */
  _plotter->position_is_unknown = true;

  /* Delete the page buffer, since HP-GL Plotters don't need to maintain a
     linked list of pages.  Each page is output individually, when its
     closepl() is invoked. */
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

  if (_plotter->outstream && fflush(_plotter->outstream) < 0)
    {
      _plotter->error ("output stream jammed");
      return -1;
    }
  else
    return 0;
}
