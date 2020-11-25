/* This file contains miscellaneous subroutines for GNU graph. */

#include "sys-defines.h"
#include "extern.h"

void
array_bounds (p, length, transpose_axes,
	      min_x, min_y, max_x, max_y,
	      spec_min_x, spec_min_y, spec_max_x, spec_max_y)
     const Point *p;
     int length;
     Boolean transpose_axes;
     double *min_x, *min_y, *max_x, *max_y;
     Boolean spec_min_x, spec_min_y, spec_max_x, spec_max_y;
{
  double orig_min_x = 0.0, orig_min_y = 0.0;
  double orig_max_x = 0.0, orig_max_y = 0.0;
  double local_min_x = 0.0, local_min_y = 0.0; 
  double local_max_x = 0.0, local_max_y = 0.0;
  Boolean point_seen = FALSE;
  int i;

  if (length == 0)
    {
      if (!spec_min_x)
	*min_x = 0.0;
      if (!spec_min_y)
	*min_y = 0.0;
      if (!spec_max_x)
	*max_x = *min_x;
      if (!spec_max_y)
	*max_y = *min_y;
      return;
    }

  if (!spec_min_x)
    local_min_x = MAXDOUBLE;
  else
    orig_min_x = *min_x;

  if (!spec_min_y)
    local_min_y = MAXDOUBLE;
  else
    orig_min_y = *min_y;

  if (!spec_max_x)
    local_max_x = -(MAXDOUBLE);
  else
    orig_max_x = *max_x;
  
  if (!spec_max_y)
    local_max_y = -(MAXDOUBLE);
  else
    orig_max_y = *max_y;      
    
  for (i = 0; i < length; i++)
    {
      double xx = (transpose_axes ? p[i].y : p[i].x);
      double yy = (transpose_axes ? p[i].x : p[i].y);

      /* ignore points that lie beyond specified bounds */
      if ((spec_min_x && xx < orig_min_x)
	  || (spec_max_x && xx > orig_max_x)
	  || (spec_min_y && yy < orig_min_y)
	  || (spec_max_y && yy > orig_max_y))
	continue;

      point_seen = TRUE;
      if (!spec_min_x)
	local_min_x = DMIN(local_min_x, xx);
      if (!spec_min_y)
	local_min_y = DMIN(local_min_y, yy);
      if (!spec_max_x)
	local_max_x = DMAX(local_max_x, xx);
      if (!spec_max_y)
	local_max_y = DMAX(local_max_y, yy);
    }

  if (!point_seen)
    local_min_x = local_min_y = local_max_x = local_max_y = 0.0;

  if (!spec_min_x)
    *min_x = local_min_x;
  if (!spec_min_y)
    *min_y = local_min_y;
  if (!spec_max_x)
    *max_x = local_max_x;
  if (!spec_max_y)
    *max_y = local_max_y;

  return;
}
