/* This file contains the havecap method, which is a GNU extension to
   libplot.  It queries the current plotter to determine whether or not it
   has a specified capability, specified by a string.

   Return value is 0/1/2 (no/yes/maybe).  If the capability is not
   recognized, the return value is 0. */


#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_havecap (const char *s)
#else
_g_havecap (s)
     const char *s;
#endif
{
  if (strcasecmp (s, "WIDE_LINES") == 0)
    return _plotter->have_wide_lines;
  else if (strcasecmp (s, "SOLID_FILL") == 0)
    return _plotter->have_solid_fill;
  else if (strcasecmp (s, "SETTABLE_BACKGROUND") == 0)
    return _plotter->have_settable_bg;
  else if (strcasecmp (s, "HERSHEY_FONTS") == 0)
    return _plotter->have_hershey_fonts;
  else if (strcasecmp (s, "PS_FONTS") == 0)
    return _plotter->have_ps_fonts;
  else if (strcasecmp (s, "PCL_FONTS") == 0)
    return _plotter->have_pcl_fonts;
  else
    return 0;
}
