/* This file contains the linedash method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the dash array used for
   subsequent drawing of paths. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_flinedash (int n, const double *dashes, double offset)
#else
_g_flinedash (n, dashes, offset)
     int n;
     const double *dashes;
     double offset;
#endif
{
  int i;

  if (!_plotter->open)
    {
      _plotter->error ("flinedash: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0.0)
      return -1;

  if (_plotter->drawstate->dash_array_len > 0)
    free (_plotter->drawstate->dash_array);
  if (n > 0)
    _plotter->drawstate->dash_array = (double *)_plot_xmalloc (n * sizeof(double));
  else
    _plotter->drawstate->dash_array = NULL;

  _plotter->drawstate->dash_array_len = n;
  for (i = 0; i < n; i++)
    _plotter->drawstate->dash_array[i] = dashes[i];
  _plotter->drawstate->dash_offset = offset;

  /* for future paths, use dash array rather than line mode */
  _plotter->drawstate->dash_array_in_effect = true;

  return 0;
}

