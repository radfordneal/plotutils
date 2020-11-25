/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For PSPlotter objects, we first output the Postscript header (including
   the bounding box).  We then output all plotted objects, which we have
   saved in the resizable outbuf structure.  We fflush the
   _plotter->outstream and reset all datastructures. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#ifndef HAVE_UNISTD_H
#include <sys/types.h>		/* if unistd.h found, included with it */
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() */
#include <time.h>		/* for ctime() */
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

extern const char *_ps_header[6]; /* in p_header.c */

int
#ifdef _HAVE_PROTOS
_p_closepl (void)
#else
_p_closepl ()
#endif
{
  double xmin, xmax, ymin, ymax;
  int i;
  time_t clock;

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
  
  if (_plotter->outstream && _plotter->page_number == 1)
  /* first page, so write Postscript header to output stream */
    {
      fprintf (_plotter->outstream, "\
%%!PS-Adobe-3.0 EPSF-3.0\n\
%%%%Creator: GNU libplot drawing library\n\
%%%%Title: PostScript plot file\n");
      fprintf (_plotter->outstream, "\
%%%%CreationDate: %s", 
	       (time(&clock), ctime(&clock)));
      fputs ("\
%%Pages: 1\n\
%%PageOrder: Ascend\n", _plotter->outstream);
      
      /* ... in particular, the bounding box for the first page */
      _get_range (&xmin, &xmax, &ymin, &ymax);
      fprintf (_plotter->outstream, "\
%%%%BoundingBox: %d %d %d %d\n",
	       IROUND(xmin - 0.5), IROUND(ymin - 0.5),
	       IROUND(xmax + 0.5), IROUND(ymax + 0.5));
      
      /* write out list of fonts used by first page */
      fputs ("\
%%DocumentNeededResources: font ", _plotter->outstream);
      for (i = 0; i < NUM_PS_FONTS; i++)
	{
	  if (_plotter->ps_font_used[i])
	    {
	      fputs (_ps_font_info[i].ps_name, _plotter->outstream);
	      fputs (" ", _plotter->outstream);
	    }
	}
      
      fputs ("\n\
%%EndComments\n\n\
%%BeginProlog\n\
%%EndProlog\n\n",
	     _plotter->outstream);
    }
      
  /* emit the page */

  if (_plotter->outstream)
    {
      fprintf (_plotter->outstream, "\
%%%%Page: %d %d\n",
     _plotter->page_number, _plotter->page_number);
      _get_range (&xmin, &xmax, &ymin, &ymax);
      fprintf (_plotter->outstream, "\
%%%%PageBoundingBox: %d %d %d %d\n",
	       IROUND(xmin - 0.5), IROUND(ymin - 0.5),
	       IROUND(xmax + 0.5), IROUND(ymax + 0.5));
      fputs ("\
%%BeginPageSetup\n", _plotter->outstream);

      /* write out the idraw PS prologue (switches to local dictionary,
	 makes many definitions) */
      for (i=0; *_ps_header[i]; i++)
	fputs (_ps_header[i], _plotter->outstream);
      fputs ("\n", _plotter->outstream);
      
      /* emit code to reencode each used ISO8859-1 font */
      for (i = 0; i < NUM_PS_FONTS; i++)
	{
	  if (_plotter->ps_font_used[i] && _ps_font_info[i].iso8859_1)
	    fprintf (_plotter->outstream, "\
/%s reencodeISO def\n",
		     _ps_font_info[i].ps_name);
	}

      fputs ("\
%%EndPageSetup\n\n", _plotter->outstream);

      /* output a brief page header (including idraw, PS directives) */
      /* N.B. `8' below is the version number of the idraw PS format we're
	 producing; see <Unidraw/Components/psformat.h> */
      fputs ("\
%I Idraw 8\n\
\n\
Begin\n\
%I b u\n\
%I cfg u\n\
%I cbg u\n\
%I f u\n\
%I p u\n\
%I t\n\
[ 1 0 0 1 0 0 ] concat\n\
/originalCTM matrix currentmatrix def\n\
/trueoriginalCTM matrix currentmatrix def\n\
\n", _plotter->outstream);

  if (_plotter->outbuf.len > 0)
    /* emit the cached Postscript for this page */
    fputs (_plotter->outbuf.base, _plotter->outstream); 

  /* output a brief page trailer (including idraw, PS directives) */
  fputs ("\
End %I eop\n\
\n\
end\n\
showpage\n\
%%PageTrailer\n\n\
%%EOF\n\n\
", _plotter->outstream);
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

  for (i = 0; i < NUM_PS_FONTS; i++)
     _plotter->ps_font_used[i] = false; /* reset for next plot */

  /* reset range bounds (i.e. BoundingBox) for next plot */
  _reset_range();

  _plotter->open = false;	/* flag device as closed */

  if (_plotter->outstream && fflush(_plotter->outstream) < 0)
	{
	  _plotter->error ("output stream jammed");
	  return -1;
	}
  else
    return 0;
}
