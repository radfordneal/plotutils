/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for Tek Plotters.  We emit terminal initialization
   codes, depending on the Tektronix type. */

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
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

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
		   _kermit_bgcolor_escapes[_plotter->kermit_bgcolor]);
	  /* select default foreground color [presumably black or off-black] */
	  fprintf (_plotter->outstream, 
		   _kermit_fgcolor_escapes[_plotter->kermit_fgcolor]);
	}
    }

  return 0;
}
