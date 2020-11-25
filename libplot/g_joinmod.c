/* This file contains the joinmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when drawing
   subsequent polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_joinmod (const char *s)
#else
_g_joinmod (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("joinmod: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->PointsInLine > 0
      || _plotter->drawstate->arc_stashed)
    _plotter->endpath(); /* flush polyline if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _plotter->default_drawstate->join_mode;

  free (_plotter->drawstate->join_mode);
  _plotter->drawstate->join_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_plotter->drawstate->join_mode, s);

  /* The following three join types are now standard. */

  if (strcmp( s, "miter") == 0)
    _plotter->drawstate->join_type = JOIN_MITER;
  else if (strcmp( s, "mitre") == 0)
    _plotter->drawstate->join_type = JOIN_MITER;
  else if (strcmp( s, "round") == 0)
    _plotter->drawstate->join_type = JOIN_ROUND;
  else if (strcmp( s, "bevel") == 0)
    _plotter->drawstate->join_type = JOIN_BEVEL;
  else
    /* don't recognize, silently switch to default mode */
    return _g_joinmod (_plotter->default_drawstate->join_mode);
  
  return 0;
}
