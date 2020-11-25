/* This file contains the linewidth routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

/* This version is special to libplotfig, since its version of the internal
   routine _device_line_width is customized for xfig.  xfig expresses line
   widths in terms of `Fig display units' rather than `Fig units'. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
flinewidth(new_line_width)
     double new_line_width;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: flinewidth() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  if (new_line_width < 0.0)	/* reset to default */
    new_line_width = _default_drawstate.line_width;

  /* set the new linewidth in the drawing state */
  _drawstate->line_width = new_line_width;

  return 0;
}

int
linewidth(new_line_width)
     int new_line_width;
{
  return flinewidth ((double)new_line_width);
}

int 
_device_line_width ()
{
  double user_line_width, det;
  int device_line_width;
  double temp;
  
  user_line_width = _drawstate->line_width;
  det = _drawstate->transform.m[0] * _drawstate->transform.m[3] 
    - _drawstate->transform.m[1] * _drawstate->transform.m[2];
  temp = sqrt(fabs(det)) * user_line_width;

  /* xfig expresses line widths in terms of `Fig display units' */
  device_line_width = IROUND (FIG_UNITS_TO_FIG_DISPLAY_UNITS (temp));

  /* don't use 0-width lines if user specified nonzero width */
  if (device_line_width == 0 && user_line_width > 0.0)
    device_line_width = 1;
  
  return device_line_width;
}
