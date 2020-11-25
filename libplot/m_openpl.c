/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_openpl (void)
#else
_m_openpl ()
#endif
{
  const char *portable_s;
  bool not_previously_opened;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* initialize certain data members from values of relevant class variables */

  portable_s = (const char *)_get_plot_param ("META_PORTABLE");
  if (strcasecmp (portable_s, "yes") == 0)
    _plotter->portable_output = true;
  else
    _plotter->portable_output = false;
  
  /* flag device as open */
  _plotter->open = true;
  not_previously_opened = !_plotter->opened;
  _plotter->opened = true;

  if (_plotter->outstream)
    {
      /* format type 0 = pre-GNU, type 1 = GNU binary, type 2 = GNU portable */
      
      if (not_previously_opened)
	/* emit metafile header, i.e. magic string */
	{
	  if (_plotter->portable_output)
	    fprintf (_plotter->outstream, "%s 2\n", PLOT_MAGIC);
	  else
	    fprintf (_plotter->outstream, "%s 1\n", PLOT_MAGIC);
	}
      
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c\n",
		 OPENPL);
      else
	putc (OPENPL, _plotter->outstream);
    }
  
  /* create drawing state, add it as the first member of the linked list;
     use generic method because we don't want to emit an op code */
  _g_savestate();			

  return 0;
}
