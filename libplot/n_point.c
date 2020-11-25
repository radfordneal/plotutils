/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

int
#ifdef _HAVE_PROTOS
_n_fpoint (R___(Plotter *_plotter) double x, double y)
#else
_n_fpoint (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  double xx, yy;
  int ixx, iyy;
  unsigned char red, green, blue;
  miGC *pGC;
  miPixel fgPixel, bgPixel, pixels[2];
  miPoint point, offset;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fpoint: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */
  
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

  /* compute background and foreground color for miGC */
  bgPixel.type = MI_PIXEL_RGB_TYPE;
  bgPixel.u.rgb[0] = _plotter->drawstate->bgcolor.red & 0xff;
  bgPixel.u.rgb[1] = _plotter->drawstate->bgcolor.green & 0xff;
  bgPixel.u.rgb[2] = _plotter->drawstate->bgcolor.blue & 0xff;
  fgPixel.type = MI_PIXEL_RGB_TYPE;
  fgPixel.u.rgb[0] = red;
  fgPixel.u.rgb[1] = green;
  fgPixel.u.rgb[2] = blue;
  pixels[0] = bgPixel;
  pixels[1] = fgPixel;

  /* construct an miGC (graphics context for the libxmi module); copy
     attributes from the Plotter's GC to it */
  pGC = miNewGC (2, pixels);
  _set_common_mi_attributes (R___(_plotter) (voidptr_t)pGC);
  
  point.x = ixx;
  point.y = iyy;
  miDrawPoints ((miPaintedSet *)_plotter->n_painted_set, 
		pGC, MI_COORD_MODE_ORIGIN, 1, &point);

  /* deallocate miGC */
  miDeleteGC (pGC);

  /* copy from painted set to canvas, and clear */
  offset.x = 0;
  offset.y = 0;
  miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->n_painted_set, 
			    (miCanvas *)_plotter->n_canvas, 
			    offset);
  miClearPaintedSet ((miPaintedSet *)_plotter->n_painted_set);

  return 0;
}
