/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_r_paint_point (S___(Plotter *_plotter))
#else
_r_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xx, yy;
  int ixx, iyy;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* convert point to floating-point device coordinates */
      xx = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yy = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      
      /* do nothing if point is outside device clipping rectangle */
      if ((xx < REGIS_DEVICE_X_MIN_CLIP)
	  || (xx > REGIS_DEVICE_X_MAX_CLIP)
	  || (yy < REGIS_DEVICE_Y_MIN_CLIP)
	  || (yy > REGIS_DEVICE_Y_MAX_CLIP))
	return;
      
      /* round to integer device (ReGIS) coordinates */
      ixx = IROUND(xx);
      iyy = IROUND(yy);
      
      /* sync ReGIS's foreground color to be the same as our pen color */
      _r_set_pen_color (S___(_plotter));

      /* output the point, as a single pixel */
      _regis_move (R___(_plotter) ixx, iyy);
      _write_string (_plotter->data, "V[]\n");

      /* update our notion of ReGIS's notion of position */
      _plotter->regis_pos.x = ixx;
      _plotter->regis_pos.y = iyy;
    }
}
