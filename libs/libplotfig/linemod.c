/* This file contains the linemod routine, which is a standard part of
   libplot.  It sets a drawing attribute: the line mode used in subsequent
   drawing operations. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
linemod (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linemod() called when graphics device not open\n");
      return -1;
    }

  endpoly();			/* flush polyline if any */

  /* invalid pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.line_mode;

  free (_drawstate->line_mode);
  _drawstate->line_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_drawstate->line_mode, s);

  if (strcmp( s, "disconnected") == 0)
    {
      _drawstate->fig_line_style = L_SOLID;
      _drawstate->fig_dash_length = 0.0;	/* ignored for L_SOLID */
      _drawstate->points_are_connected = FALSE;
    }

  /* The following five line types have long been standard in the plot
     library.  Fig format supports only dotted lines (isolated dots, with a
     specified inter-dot distance) and dashed lines (on/off dashes, the
     lengths of the on and off segments being equal).  We map our canonical
     five line types into Fig line types as best we can. */

  else if (strcmp( s, "solid") == 0)
    {
      _drawstate->fig_line_style = L_SOLID;
      _drawstate->fig_dash_length = 0.0;	/* ignored for L_SOLID */
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "dotted") == 0)
    {
      _drawstate->fig_line_style = L_DOTTED;
      _drawstate->fig_dash_length = 2.0; /* i.e. inter-dot length */
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "dotdashed") == 0) /* sigh... */
    {
      _drawstate->fig_line_style = L_DOTTED;
      _drawstate->fig_dash_length = 4.0; /* i.e. inter-dot length */
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "shortdashed") == 0)
    {
      _drawstate->fig_line_style = L_DASHED;
      _drawstate->fig_dash_length = 4.0; /* length of on and off segments */
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "longdashed") == 0)
    {
      _drawstate->fig_line_style = L_DASHED;
      _drawstate->fig_dash_length = 8.0; /* length of on and off segments */
      _drawstate->points_are_connected = TRUE;
    }
  else
    /* don't recognize, silently switch to default mode */
    linemod (_default_drawstate.line_mode);
  
  return 0;
}
