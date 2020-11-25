/* This file contains the capmod routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the cap mode used when drawing
   subsequent lines and polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
capmod (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: capmod() called when graphics device not open\n");
      return -1;
    }

  endpoly();			/* flush polyline if any */

  /* invalid pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.cap_mode;

  free (_drawstate->cap_mode);
  _drawstate->cap_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_drawstate->cap_mode, s);

  return 0;
}
