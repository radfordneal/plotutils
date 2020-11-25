/* This file contains the closepl routine, which is a standard part of
   libplot.  It closes the graphics device.

   For libplotps, we first output the Postscript header (including the
   bounding box).  We then output all plotted objects, which we have saved
   in the resizable outbuf structure.  We fflush the _outstream and reset
   all datastructures; the user might want to call outfile(), so as to draw
   another plot. */

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

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE	/* Needed for pwd.h by `c89' on AIX 4.1, at least */
#endif
#include <pwd.h>		/* for getpwuid(), struct passwd */

extern char *header[6];

int
closepl ()
{
  double xmin, xmax, ymin, ymax;
  int i;
  struct passwd *pwd;
  time_t clock;
#ifdef HAVE_GETHOSTNAME
  char hostname[256];
#endif

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: closepl() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  pwd = getpwuid(getuid());
#ifdef HAVE_GETHOSTNAME
  gethostname(hostname, sizeof (hostname));
#endif

   /* begin writing Postscript header to output stream */
  fputs ("\
%!PS-Adobe-3.0 EPSF-3.0\n", _outstream);
#ifdef HAVE_GETHOSTNAME
  fprintf(_outstream, 
	  "%%%%Creator: %s@%s (%s)\n", pwd->pw_name, hostname, pwd->pw_gecos);
#else
  fprintf(_outstream, "%%%%Creator: %s (%s)\n", pwd->pw_name, pwd->pw_gecos);
#endif
  fprintf(_outstream, "%%%%Title: PostScript plot file\n");
  fprintf(_outstream, "%%%%CreationDate: %s", (time(&clock), ctime(&clock)));
  fputs ("\
%%Pages: 1\n\
%%PageOrder: Ascend\n", _outstream);

  /* ... in particular, the bounding box.  (In terms of device units, which
     on account of SCALING may not be the same as points.  In fact, one
     device unit = SCALING points.) */
  _get_range (&xmin, &xmax, &ymin, &ymax);
  fprintf (_outstream, "%%%%BoundingBox: %d %d %d %d\n",
	   IROUND(DEVICE_UNITS_TO_POINTS(xmin) - 0.5),
	   IROUND(DEVICE_UNITS_TO_POINTS(ymin) - 0.5),
	   IROUND(DEVICE_UNITS_TO_POINTS(xmax) + 0.5),
	   IROUND(DEVICE_UNITS_TO_POINTS(ymax) + 0.5));
  
  /* write out list of fonts used */
  fputs ("\
%%DocumentFonts: ", _outstream);
  i = -1;
  while (_ps_font_info[++i].ps_name)
    {
      if (_ps_font_info[i].used)
	{
	  fputs (_ps_font_info[i].ps_name, _outstream);
	  fputs (" ", _outstream);
	}
    }
  fputs ("\n", _outstream);

  fputs ("%%EndComments\n\n\
%%BeginProcSet: plotps.pro\n", 
	 _outstream);

  /* write the standard prologue; this includes the CTM, which includes
     the SCALING factor */
  for (i=0; *header[i]; i++)
    fputs (header[i], _outstream);

  fputs ("\
%%EndProcSet\n\
%%EndProlog\n\
\n", _outstream);

  /* reencode each used ISO8859-1 font */
  fputs ("\
%%BeginSetup\n", _outstream);
  i = -1;
  while (_ps_font_info[++i].ps_name)
    {
      if (_ps_font_info[i].used && _ps_font_info[i].iso8859_1)
	fprintf (_outstream, "\
/%s reencodeISO def\n",
		 _ps_font_info[i].ps_name);
      _ps_font_info[i].used = FALSE; /* reset for next plot */
    }
  fputs ("\
%%EndSetup\n\
\n", _outstream);

  /* `8' below is the version number of the idraw PS format we're
     producing; see <Unidraw/Components/psformat.h> */
  fputs ("\
%%Page: 1 1\n\
\n\
%I Idraw 8\n\
\n\
Begin\n\
%I b u\n\
%I cfg u\n\
%I cbg u\n\
%I f u\n\
%I p u\n\
%I t\n[ ", _outstream);
  fprintf (_outstream, "%f 0 0 %f", SCALING, SCALING);
  fputs (" 0 0 ] concat\n\
/originalCTM matrix currentmatrix def\n\
\n", _outstream);

  if (_outbuf.len > 0)		/* output all the cached Postscript */
    fputs (_outbuf.base, _outstream); 

  fputs ("\
End %I eop\n\
\n\
%%Trailer\n\
", _outstream);
  /* if we had an `atend' trailer it would go here */
  fputs ("\
\n\
end\n\
showpage\n\
%%EOF\n\
", _outstream);

  fflush (_outstream);

  free (_outbuf.base);		/* free output buffer */

  if (_drawstate->previous != NULL)
    {
      fprintf (stderr, "libplot: closepl() called at stack state depth >1\n");
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
