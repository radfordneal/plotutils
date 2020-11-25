/* This file contains the fillmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the fill rule used when
   subsequently drawing filled objects, i.e., the rule used to determine
   which points are `inside'.

   In principle, both the `odd winding number' rule and the `nonzero
   winding number' rule are supported.  The former is the default. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fillmod (const char *s)
#else
_g_fillmod (s)
     const char *s;
#endif
{
  const char *default_s;

  if (!_plotter->open)
    {
      _plotter->error ("fillmod: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* determine default fill rule (can't just read from default drawing
     state, because not all Plotters support both standard rules) */
  default_s = _default_drawstate.fill_rule;
  if (strcmp (default_s, "even-odd") == 0
      && _plotter->have_odd_winding_fill == 0)
    default_s = "nonzero-winding";
  else if (strcmp (default_s, "nonzero-winding") == 0
	   && _plotter->have_nonzero_winding_fill == 0)
    default_s = "even-odd";
  
  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = default_s;

  free (_plotter->drawstate->fill_rule);
  _plotter->drawstate->fill_rule = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_plotter->drawstate->fill_rule, s);

  if ((strcmp (s, "even-odd") == 0 || strcmp (s, "alternate") == 0)
      && _plotter->have_odd_winding_fill)
    _plotter->drawstate->fill_rule_type = FILL_ODD_WINDING;
  else if ((strcmp (s, "nonzero-winding") == 0 || strcmp (s, "winding") == 0)
	   && _plotter->have_nonzero_winding_fill)
    _plotter->drawstate->fill_rule_type = FILL_NONZERO_WINDING;
  else
    /* don't recognize, silently switch to default fill rule */
    _g_fillmod (default_s);

  return 0;
}
