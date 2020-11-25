/* This file contains the joinmod routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when drawing
   subsequent polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
joinmod (s)
     char *s;
{
  XGCValues gcv;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: joinmod() called when graphics device not open\n");
      return -1;
    }

  endpoly();			/* flush polyline if any */

  /* invalid pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.join_mode;

  free (_drawstate->join_mode);
  _drawstate->join_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_drawstate->join_mode, s);

  /* The following three join types are now standard. */

  if (strcmp( s, "miter") == 0)
    gcv.join_style = JoinMiter;
  else if (strcmp( s, "round") == 0)
    gcv.join_style = JoinRound;
  else if (strcmp( s, "bevel") == 0)
    gcv.join_style = JoinBevel;
  else
    {
      /* don't recognize, silently switch to default mode */
      joinmod (_default_drawstate.line_mode);
      return 0;
    }
  
  XChangeGC (_xdata.dpy, _drawstate->gc, GCJoinStyle, &gcv);

  _handle_x_events();

  return 0;
}
