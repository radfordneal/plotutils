/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

void
#ifdef _HAVE_PROTOS
_i_paint_point (S___(Plotter *_plotter))
#else
_i_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xx, yy;
  int ixx, iyy;
  miGC *pGC;
  miPixel fgPixel, bgPixel, pixels[2];
  miPoint point, offset;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* convert point to floating-point device coordinates */
      xx = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yy = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      
      /* round to integer device (GIF) coordinates */
      ixx = IROUND(xx);
      iyy = IROUND(yy);
      
      /* compute background and foreground color for miGC */
      _i_set_pen_color (S___(_plotter));
      bgPixel.type = MI_PIXEL_INDEX_TYPE;
      bgPixel.u.index = _plotter->drawstate->i_bg_color_index;
      fgPixel.type = MI_PIXEL_INDEX_TYPE;
      fgPixel.u.index = _plotter->drawstate->i_pen_color_index;
      pixels[0] = bgPixel;
      pixels[1] = fgPixel;
      
      /* construct an miGC (graphics context for the libxmi module); copy
	 attributes from the Plotter's GC to it */
      pGC = miNewGC (2, pixels);
      _set_common_mi_attributes (_plotter->drawstate, (voidptr_t)pGC);
      
      point.x = ixx;
      point.y = iyy;
      miDrawPoints ((miPaintedSet *)_plotter->i_painted_set, 
		    pGC, MI_COORD_MODE_ORIGIN, 1, &point);
      
      /* deallocate miGC */
      miDeleteGC (pGC);
      
      /* copy from painted set to canvas, and clear */
      offset.x = 0;
      offset.y = 0;
      miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->i_painted_set, 
				(miCanvas *)_plotter->i_canvas, 
				offset);
      miClearPaintedSet ((miPaintedSet *)_plotter->i_painted_set);
      
      /* something was drawn in frame */
      _plotter->i_frame_nonempty = true;
    }
}
