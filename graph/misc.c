/* This file contains miscellaneous subroutines for GNU graph. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
array_bounds (const Point *p, int length, bool transpose_axes,
	      double *min_x, double *min_y, double *max_x, double *max_y,
	      bool spec_min_x, bool spec_min_y, 
	      bool spec_max_x, bool spec_max_y)
#else
array_bounds (p, length, transpose_axes,
	      min_x, min_y, max_x, max_y,
	      spec_min_x, spec_min_y, spec_max_x, spec_max_y)
     const Point *p;
     int length;
     bool transpose_axes;
     double *min_x, *min_y, *max_x, *max_y;
     bool spec_min_x, spec_min_y, spec_max_x, spec_max_y;
#endif
{
  double orig_min_x = 0.0, orig_min_y = 0.0;
  double orig_max_x = 0.0, orig_max_y = 0.0;
  double local_min_x = 0.0, local_min_y = 0.0; 
  double local_max_x = 0.0, local_max_y = 0.0;
  bool point_seen = false;
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

  if (spec_min_x)
    orig_min_x = *min_x;
  else
    local_min_x = DBL_MAX;

  if (spec_max_x)
    orig_max_x = *max_x;
  else
    local_max_x = -(DBL_MAX);
  
  /* special case: user specified both limits, but min > max (reversed axis) */
  if (spec_min_x && spec_max_x && orig_min_x > orig_max_x)
    {
      double tmp;
      
      tmp = orig_min_x;
      orig_min_x = orig_max_x;
      orig_max_x = tmp;
    }

  if (spec_min_y)
    orig_min_y = *min_y;
  else
    local_min_y = DBL_MAX;

  if (spec_max_y)
    orig_max_y = *max_y;      
  else
    local_max_y = -(DBL_MAX);
    
  /* special case: user specified both limits, but min > max (reversed axis) */
  if (spec_min_y && spec_max_y && orig_min_y > orig_max_y)
    {
      double tmp;
      
      tmp = orig_min_y;
      orig_min_y = orig_max_y;
      orig_max_y = tmp;
    }

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

      point_seen = true;
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
