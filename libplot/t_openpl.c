/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_t_openpl (void)
#else
_t_openpl ()
#endif
{
  const char* term_type;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* initialize data members from values of relevant class variables */

  term_type = (const char *)_get_plot_param ("TERM");
  if (term_type != NULL)
    {
      if ((strcmp (term_type, "xterm") == 0)
	  || (strcmp (term_type, "xterms") == 0)
	  || (strcmp (term_type, "kterm") == 0))
	_plotter->display_type = D_XTERM;
      else if ((strcmp (term_type, "ansi.sys") == 0)
	       || (strcmp (term_type, "ansi.sysk") == 0)
	       || (strcmp (term_type, "ansisys") == 0)
	       || (strcmp (term_type, "ansisysk") == 0)
	       || (strcmp (term_type, "kermit") == 0))
	_plotter->display_type = D_KERMIT;
      else
	_plotter->display_type = D_GENERIC;
    }
  else
    _plotter->display_type = D_GENERIC;
  
  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* send graphics initialization commands to output stream if necessary */

  if (_plotter->outstream)
    {
      if (_plotter->display_type == D_XTERM 
	  || _plotter->display_type == D_KERMIT)
	/* VT340 command, put xterm / kermit into Tek mode.*/
	fprintf (_plotter->outstream, "\033[?38h");
    }

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  if (_plotter->outstream)
    {
      if (_plotter->display_type == D_KERMIT)
	{
	  /* select default background color [presumably white or off-white] */
	  fprintf (_plotter->outstream,
		   _kermit_bgcolor_escapes[_tek_default_plotter.kermit_bgcolor]);
	  /* select default foreground color [presumably black or off-black] */
	  fprintf (_plotter->outstream, 
		   _kermit_fgcolor_escapes[_tek_default_plotter.kermit_fgcolor]);
	}
    }

  return 0;
}
