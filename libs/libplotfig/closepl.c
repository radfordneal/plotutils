/* This file is the closepl routine, which is a standard part of libplot.
   It closes the graphics device.

   For libplotfig, we first output all user-defined colors [``color
   pseudo-objects''], which must appear first in the .fig file.  We then
   output all genuine objects, which we have saved in the resizable outbuf
   structure.  Finally we fflush the _outstream and reset all
   datastructures; the user might want to call outfile(), so as to draw
   another plot.  We do not close the _outstream. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
closepl ()
{
  int i;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: closepl() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  fprintf(_outstream, "#FIG 3.1\n%s\n%s\n%s\n%d %d\n",
	  "Portrait",		/* portrait mode */
	  "Center",		/* justification */
	  "Inches",		
	  FIG_RESOLUTION,	/* Fig units per inch */
	  2			/* origin in lower left corner (ignored) */
	  );
  fflush(_outstream);

  for (i = 0; i < _num_usercolors; i++) /* output user-defined colors if any */
    {
      fprintf(_outstream, 
	      "#COLOR\n%d %d #%06lx\n",
	      0,		/* color pseudo-object */
	      FIG_USER_COLOR_MIN + i,  /* color number, in xfig's range */
	      _usercolors[i]	/* 24-bit RGB value */
	      );
    }

  if (_outbuf.contents > 0)	/* output all cached objects */
    fputs (_outbuf.base, _outstream); 

  fflush (_outstream);

  free (_outbuf.base);		/* free output buffer */

  _num_usercolors = 0;		/* forget user-defined colors */
  
  if (_drawstate->previous != NULL)
    {
      fprintf (stderr, "libplot: closepl() called at state stack depth >1\n");
      while (_drawstate->previous)
	restorestate();		/* pop extraneous state off the stack */
    }
  
  /* remove the zeroth state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_drawstate->line_mode);
  free (_drawstate->join_mode);
  free (_drawstate->cap_mode);
  free (_drawstate->font_name);
  
  free (_drawstate);
  _drawstate = NULL;

  _grdevice_open = FALSE;	/* flag device as closed */

  return 0;
}
