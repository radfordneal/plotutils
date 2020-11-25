/* This file contains the havecap method, which is a GNU extension to
   libplot.  It queries the current plotter to determine whether or not it
   has a specified capability, specified by a string.

   Return value is 0/1/2 (no/yes/maybe).  If the capability is not
   recognized, the return value is 0. */


#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_havecap (R___(Plotter *_plotter) const char *s)
#else
_API_havecap (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  if (strcasecmp (s, "WIDE_LINES") == 0)
    return _plotter->data->have_wide_lines;
  else if (strcasecmp (s, "SOLID_FILL") == 0)
    return _plotter->data->have_solid_fill;
  else if (strcasecmp (s, "DASH_ARRAY") == 0)
    return _plotter->data->have_dash_array;
  else if (strcasecmp (s, "EVEN_ODD_FILL") == 0)
    return _plotter->data->have_odd_winding_fill;
  else if (strcasecmp (s, "NONZERO_WINDING_NUMBER_FILL") == 0)
    return _plotter->data->have_nonzero_winding_fill;
  else if (strcasecmp (s, "SETTABLE_BACKGROUND") == 0)
    return _plotter->data->have_settable_bg;
  else if (strcasecmp (s, "HERSHEY_FONTS") == 0)
    return 1;			/* always supported */
  else if (strcasecmp (s, "PS_FONTS") == 0)
    return _plotter->data->have_ps_fonts;
  else if (strcasecmp (s, "PCL_FONTS") == 0)
    return _plotter->data->have_pcl_fonts;
  else if (strcasecmp (s, "STICK_FONTS") == 0)
    return _plotter->data->have_stick_fonts;
  else if (strcasecmp (s, "EXTRA_STICK_FONTS") == 0)
    return _plotter->data->have_extra_stick_fonts;
  else
    return 0;
}
