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
  XGCValues gcv;
  
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

  /* The following three cap types are now standard. */

  if (strcmp( s, "butt") == 0)
    gcv.cap_style = CapButt;
  else if (strcmp( s, "round") == 0)
    gcv.cap_style = CapRound;
  else if (strcmp( s, "projecting") == 0)
    gcv.cap_style = CapProjecting;
  else
    {
      /* don't recognize, silently switch to default mode */
      capmod (_default_drawstate.cap_mode);
      return 0;
    }
  
  XChangeGC (_xdata.dpy, _drawstate->gc, GCCapStyle, &gcv);

  _handle_x_events();

  return 0;
}
