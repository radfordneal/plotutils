/* This file contains the joinmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when
   subsequently drawing paths consisting of more than a single segment. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_joinmod (R___(Plotter *_plotter) const char *s)
#else
_g_joinmod (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  char *join_mode;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "joinmod: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.join_mode;

  free ((char *)_plotter->drawstate->join_mode);
  join_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (join_mode, s);
  _plotter->drawstate->join_mode = join_mode;

  /* The following four join types are now standard. */

  if (strcmp( s, "miter") == 0)
    _plotter->drawstate->join_type = JOIN_MITER;
  else if (strcmp( s, "mitre") == 0)
    _plotter->drawstate->join_type = JOIN_MITER;
  else if (strcmp( s, "round") == 0)
    _plotter->drawstate->join_type = JOIN_ROUND;
  else if (strcmp( s, "bevel") == 0)
    _plotter->drawstate->join_type = JOIN_BEVEL;
  else if (strcmp( s, "triangular") == 0)
    _plotter->drawstate->join_type = JOIN_TRIANGULAR;
  else
    /* unknown, so silently switch to default mode (via recursive call) */
    return _g_joinmod (R___(_plotter) _default_drawstate.join_mode);
  
  return 0;
}
