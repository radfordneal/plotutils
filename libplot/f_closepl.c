/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For FigPlotter objects, we first output all user-defined colors [``color
   pseudo-objects''], which must appear first in the .fig file.  We then
   output all genuine objects, which we have saved in a resizable outbuf
   structure for the current page.  Finally we fflush the output stream and
   reset all datastructures.

   All this applies only if this is page #1, since a Fig file may contain
   no more than a single page of graphics.  Later pages are simply
   deallocated. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_f_closepl (S___(Plotter *_plotter))
#else
_f_closepl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i, retval;
  
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate (S___(_plotter));
    }
  
  /* Output the page, but only if it's page #1 (currently Fig format
     supports only one page of graphics output per file). */
  if (_plotter->page_number == 1)
    {
      const char *units;
      plOutbuf *fig_header;
      
      /* prepare Fig header, write it to a plOutbuf */
      fig_header = _new_outbuf ();
      
      units = (_plotter->page_data->metric ? "Metric" : "Inches");
      sprintf (fig_header->point,
	       "#FIG 3.2\n%s\n%s\n%s\n%s\n%.2f\n%s\n%d\n%d %d\n",
	       "Portrait",	/* portrait mode, not landscape */
	       "Flush Left",	/* justification */
	       units,		/* "Metric" or "Inches" */
	       _plotter->page_data->fig_name, /* paper size */
	       100.00,		/* export and print magnification */
	       "Single",	/* "Single" or "Multiple" pages */
	       -2,		/* color number for transparent color */
	       IROUND(FIG_UNITS_PER_INCH), /* Fig units per inch */
	       2		/* origin in lower left corner (ignored) */
	       );
      _update_buffer (fig_header);
      
      /* output user-defined colors if any */
      for (i = 0; i < _plotter->fig_num_usercolors; i++)
	{
	  sprintf (fig_header->point,
		   "#COLOR\n%d %d #%06lx\n",
		   0,	               /* color pseudo-object */
		   FIG_USER_COLOR_MIN + i, /* color num, in xfig's range */
		   _plotter->fig_usercolors[i] /* 24-bit RGB value */
		   );
	  _update_buffer (fig_header);
	}
	  
      /* WRITE HEADER (and free its plOutbuf) */
      _plotter->write_string (R___(_plotter) fig_header->base); 
      _delete_outbuf (fig_header);
      
      /* WRITE OUT ALL FIG OBJECTS */
      if (_plotter->page->len > 0)
	_plotter->write_string (R___(_plotter) _plotter->page->base); 
    }
  
  /* Delete the page buffer, since Fig Plotters don't maintain a linked
     list of pages. */
  _delete_outbuf (_plotter->page);
  _plotter->page = NULL;

  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  /* attempt to flush (will test whether stream is jammed) */
  retval = _plotter->flushpl (S___(_plotter));

  _plotter->open = false;	/* flag device as closed */

  return retval;
}
