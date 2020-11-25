/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

void
#ifdef _HAVE_PROTOS
_b_paint_point (S___(Plotter *_plotter))
#else
_b_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xx, yy;
  int ixx, iyy;
  unsigned char red, green, blue;
  miGC *pGC;
  miPixel fgPixel, bgPixel, pixels[2];
  miPoint point, offset;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* convert point to floating-point device coordinates */
      xx = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yy = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      
      /* round to integer device coordinates */
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
      _set_common_mi_attributes (_plotter->drawstate, (voidptr_t)pGC);
      
      point.x = ixx;
      point.y = iyy;
      miDrawPoints ((miPaintedSet *)_plotter->b_painted_set, 
		    pGC, MI_COORD_MODE_ORIGIN, 1, &point);
      
      /* deallocate miGC */
      miDeleteGC (pGC);
      
      /* copy from painted set to canvas, and clear */
      offset.x = 0;
      offset.y = 0;
      miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->b_painted_set, 
				(miCanvas *)_plotter->b_canvas, 
				offset);
      miClearPaintedSet ((miPaintedSet *)_plotter->b_painted_set);
    }
}
