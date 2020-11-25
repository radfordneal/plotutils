/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For FigPlotter objects, we first output all user-defined colors [``color
   pseudo-objects''], which must appear first in the .fig file.  We then
   output all genuine objects, which we have saved in a resizable outbuf
   structure for the current page.  Finally we fflush the
   _plotter->outstream and reset all datastructures.

   All this applies only if this is page #1, since a Fig file may contain
   no more than a single page of graphics.  Later pages are simply
   deallocated. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_f_closepl (void)
#else
_f_closepl ()
#endif
{
  int i;

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
  
  /* Output the page, but only if it's page #1 (currently Fig format
     supports only one page of graphics output per file). */
  if (_plotter->page_number == 1 && _plotter->outstream)
    {
      const char *units;

      units = (_plotter->use_metric ? "Metric" : "Inches");
      fprintf(_plotter->outstream, "#FIG 3.2\n%s\n%s\n%s\n%s\n%.2f\n%s\n%d\n%d %d\n",
	      "Portrait",	/* portrait mode */
	      "Center",		/* justification */
	      units,
	      _plotter->page_type,
	      100.00,
	      "Single",
	      -2,
	      IROUND(FIG_UNITS_PER_INCH), /* Fig units per inch */
	      2			/* origin in lower left corner (ignored) */
	      );
      fflush(_plotter->outstream);
      
      /* output user-defined colors if any */
      for (i = 0; i < _plotter->fig_num_usercolors; i++)
	{
	  fprintf(_plotter->outstream, 
		  "#COLOR\n%d %d #%06lx\n",
		  0,		/* color pseudo-object */
		  FIG_USER_COLOR_MIN + i,  /* color number, in xfig's range */
		  _plotter->fig_usercolors[i] /* 24-bit RGB value */
		  );
	}
      
      if (_plotter->page->len > 0) /* output all cached objects */
	fputs (_plotter->page->base, _plotter->outstream); 
    }
  
  /* Delete the page buffer, since Fig Plotters don't need to maintain a
     linked list of pages. */
  _delete_outbuf (_plotter->page);
  _plotter->page = NULL;

  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  /* reset our knowledge of xfig's internal state (necessary only if user
     calls outfile() to reuse the Plotter) */
  _plotter->fig_drawing_depth = FIG_INITIAL_DEPTH;
  _plotter->fig_num_usercolors = 0;

  _plotter->open = false;	/* flag device as closed */

  if (_plotter->outstream && fflush(_plotter->outstream) < 0)
	{
	  _plotter->error ("output stream jammed");
	  return -1;
	}
  else
    return 0;
}
