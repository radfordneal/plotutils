/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_erase (void)
#else
_m_erase ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("erase: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c\n", 
		 (int)O_ERASE);
      else
	putc ((int)O_ERASE, _plotter->outstream);
    }

  return 0;
}
