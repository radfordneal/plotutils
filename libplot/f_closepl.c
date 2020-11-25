/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For FigPlotter objects, we first output all user-defined colors [``color
   pseudo-objects''], which must appear first in the .fig file.  We then
   output all genuine objects, which we have saved in the resizable outbuf
   structure.  Finally we fflush the _plotter->outstream and reset all
   datastructures. */

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
  
  if (_plotter->outstream)
    {
      const char *units;

      units = (_plotter->fig_use_metric ? "Metric" : "Inches");
      fprintf(_plotter->outstream, "#FIG 3.1\n%s\n%s\n%s\n%d %d\n",
	      "Portrait",	/* portrait mode */
	      "Center",		/* justification */
	      units,
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
      
      if (_plotter->outbuf.contents > 0) /* output all cached objects */
	fputs (_plotter->outbuf.base, _plotter->outstream); 
    }
  
  free (_plotter->outbuf.base);		/* free output buffer */

  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  /* reset our knowledge of xfig's internal state */
  _plotter->fig_drawing_depth = FIG_INITIAL_DEPTH;
  _plotter->fig_last_priority = 0;
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
