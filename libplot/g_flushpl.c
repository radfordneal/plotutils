/* This file contains the flushpl method, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_flushpl (void)
#else
_g_flushpl ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("flushpl: invalid operation");
      return -1;
    }

  if (_plotter->outstream == NULL)
    return 0;

  if (fflush(_plotter->outstream) < 0)
	{
	  _plotter->error ("output stream jammed");
	  return -1;
	}
  else
    return 0;
}
