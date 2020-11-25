/* This file, which is part of libplotps, contains the get_range and
   set_range routines.  They set and return the range of output coordinates
   in device units, for use in an EPS boundingbox comment.  Note 1 device
   unit = SCALING points; SCALING is defined in extern.h. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double _x_max, _x_min;
double _y_max, _y_min;

void 
_set_range (x, y)
     double x, y;
{
  if (x > _x_max) _x_max = x;
  if (x < _x_min) _x_min = x;
  if (y > _y_max) _y_max = y;
  if (y < _y_min) _y_min = y;
}

void 
_reset_range ()		/* called by openpl(), erase() */
{
  _x_max = -(MAXDOUBLE);
  _x_min = MAXDOUBLE;
  _y_max = -(MAXDOUBLE);
  _y_min = MAXDOUBLE;
}

void 
_get_range (xmin, xmax, ymin, ymax)
     double *xmin, *xmax, *ymin, *ymax;
{
  if ((_x_max < _x_min) || (_y_max < _y_min)) /* no objects */
    {
      *xmax = 0.0;
      *xmin = 0.0;
      *ymax = 0.0;
      *ymin = 0.0;
    }
  else
    {
      *xmax = _x_max;
      *xmin = _x_min;
      *ymax = _y_max;
      *ymin = _y_min;
    }
}
