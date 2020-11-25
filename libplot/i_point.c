/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* A GIFPlotter displays a point by setting a single pixel, without using
   the MI scan conversion module that it uses for rendering paths (see
   i_endpath.c). */

#include "sys-defines.h"
#include "extern.h"

#define SETPIXEL(x,y,color_index)   _plotter->i_bitmap[y][x].index=color_index

int
#ifdef _HAVE_PROTOS
_i_fpoint (double x, double y)
#else
_i_fpoint (x, y)
     double x, y;
#endif
{
  double xx, yy;
  int ixx, iyy;

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

  /* round to integer device (GIF) coordinates */
  ixx = IROUND(xx);
  iyy = IROUND(yy);

  /* set color index in drawing state */
  _plotter->set_pen_color ();
  
  /* output the point as a single pixel */
  if (ixx >= 0 && ixx < _plotter->i_xn && iyy >= 0 && iyy < _plotter->i_yn)
    SETPIXEL(ixx,iyy,_plotter->drawstate->i_pen_color_index);

  /* something was drawn in frame */
  _plotter->i_frame_nonempty = true;

  return 0;
}
