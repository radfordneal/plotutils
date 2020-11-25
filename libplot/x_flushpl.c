/* This file contains the flushpl method, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_flushpl (void)
#else
_x_flushpl ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("flushpl: invalid operation");
      return -1;
    }

  /* if (_plotter->type == PL_X11) */
    {
      XSync (_plotter->dpy, (Bool)false);
      _handle_x_events();
    }

  return 0;
}
