/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* A PNMPlotter displays a point by setting a single pixel, without using
   the MI scan conversion module that it uses for rendering paths (see
   n_endpath.c). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_n_fpoint (double x, double y)
#else
_n_fpoint (x, y)
     double x, y;
#endif
{
  double xx, yy;
  int ixx, iyy;
  unsigned char red, green, blue;

  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */
  
  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  /* convert point to floating-point device coordinates */
  xx = XD(x,y);
  yy = YD(x,y);

  /* round to integer device (PNM) coordinates */
  ixx = IROUND(xx);
  iyy = IROUND(yy);

  /* compute 24-bit color */
  red = ((unsigned int)(_plotter->drawstate->fgcolor.red) >> 8) & 0xff;
  green = ((unsigned int)(_plotter->drawstate->fgcolor.green) >> 8) & 0xff;
  blue = ((unsigned int)(_plotter->drawstate->fgcolor.blue) >> 8) & 0xff;  

  /* output the point as a single pixel */
  _plotter->n_bitmap[iyy][ixx].rgb[0] = red;
  _plotter->n_bitmap[iyy][ixx].rgb[1] = green;
  _plotter->n_bitmap[iyy][ixx].rgb[2] = blue;

  return 0;
}
