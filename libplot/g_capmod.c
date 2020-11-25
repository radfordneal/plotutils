/* This file contains the capmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the cap mode used when
   subsequently drawing open paths. */

/* In this generic version, we simply store the new cap mode name in the
   drawing state. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_capmod (const char *s)
#else
_g_capmod (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("capmod: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.cap_mode;

  free (_plotter->drawstate->cap_mode);
  _plotter->drawstate->cap_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_plotter->drawstate->cap_mode, s);

  /* The following three cap types are now standard. */

  if (strcmp( s, "butt") == 0)
    _plotter->drawstate->cap_type = CAP_BUTT;
  else if (strcmp( s, "round") == 0)
    _plotter->drawstate->cap_type = CAP_ROUND;
  else if (strcmp( s, "projecting") == 0)
    _plotter->drawstate->cap_type = CAP_PROJECT;
  else if (strcmp( s, "triangular") == 0)
    _plotter->drawstate->cap_type = CAP_TRIANGULAR;
  else
    /* don't recognize, silently switch to default mode */
    return _g_capmod (_default_drawstate.cap_mode);
  
  return 0;
}
