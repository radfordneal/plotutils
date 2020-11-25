/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"		/* use libxmi scan conversion module */

/* In this version, we first check to see if the angle of inclination is
   zero, and the map from the user frame to the device frame preserves
   axes.  If these are both true, we can use the libxmi module to draw the
   ellipse.  If they are not, we draw an inscribed polyline, by invoking
   the generic method. */

int
#ifdef _HAVE_PROTOS
_i_fellipse (R___(Plotter *_plotter) double xc, double yc, double rx, double ry, double angle)
#else
_i_fellipse (R___(_plotter) xc, yc, rx, ry, angle)
     S___(Plotter *_plotter;)
     double xc, yc, rx, ry, angle;
#endif
{
  int ninetymult = IROUND(angle / 90.0);
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fellipse: invalid operation");
      return -1;
    }

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
      /* just move to center (a libplot convention), flushing path if any */
      _plotter->fmove (R___(_plotter) xc, yc);
      return 0;
    }
  
  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter));	/* flush path if any */

 /* if angle is multiple of 90 degrees, modify to permit use of libxmi's
    arc rendering */
  if (angle == (double) (90 * ninetymult))
    {
      angle = 0.0;
      if (ninetymult % 2)
	{
	  double temp;
	  
	  temp = rx;
	  rx = ry;
	  ry = temp;
	}
    }

  rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure libxmi problems */
  ry = (ry < 0.0 ? -ry : ry);  

  /* axes flipped? (by default y-axis is, due to xmi's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);
  
  /* location of `origin' (upper left corner of bounding rect. for ellipse)
     and width and height; libxmi's flipped-y convention affects these
     values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
  
  /* If ellipse must be drawn at an angle, or affine map from user frame to
     device frame does not preserve coordinate axes, call the generic class
     method to draw an inscribed polyline.

     Do the same if squaresize_{x|y} is zero, which is a case that libxmi
     can't handle (see below). */

  if (angle != 0.0 || !(_plotter->drawstate->transform.axes_preserved)
      || squaresize_x == 0 || squaresize_y == 0)
    {
      int retval;

      /* draw, move to center, flush out */
      retval = _g_fellipse (R___(_plotter) xc, yc, rx, ry, angle); 
      return retval;
    }

  /* otherwise use libxmi scan-conversion module, as follows */

  /* draw ellipse (elliptic arc aligned with the coordinate axes, arc range
     = 64*360 64'ths of a degree) */
  _i_draw_elliptic_mi_arc_internal (R___(_plotter)
				    xorigin, yorigin, 
				    squaresize_x, squaresize_y, 
				    0, 64 * 360);
  
  _plotter->drawstate->pos.x = xc; /* move to center (a libplot convention) */
  _plotter->drawstate->pos.y = yc;
  
  return 0;
}

/* Draw an elliptic arc aligned with the coordinate axes, by invoking a
   function in the libxmi API.  Takes account of the possible need for
   filling.

   Only the case squaresize_x > 0 and squaresize_y > 0 is handled, since
   miFillArcs() and miDrawArcs() do not support squaresize_x = 0 or
   squaresize_y = 0. */

void
#ifdef _HAVE_PROTOS
_i_draw_elliptic_mi_arc_internal (R___(Plotter *_plotter) int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
#else
_i_draw_elliptic_mi_arc_internal (R___(_plotter) xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange)
     S___(Plotter *_plotter;)
     int xorigin, yorigin; 
     unsigned int squaresize_x, squaresize_y; 
     int startangle, anglerange;
#endif
{
  /* sanity check */
  if (squaresize_x == 0 || squaresize_y == 0)
    return;

  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* have something to draw */
    {
      miGC *pGC;
      miArc arc;
      miPixel fgPixel, bgPixel;
      miPixel pixels[2];
      miPoint offset;

      /* determine background pixel color */
      bgPixel.type = MI_PIXEL_INDEX_TYPE;
      bgPixel.u.index = _plotter->drawstate->i_bg_color_index;
      pixels[0] = bgPixel;
      pixels[1] = bgPixel;
      
      /* construct an miGC (graphics context for the libxmi module); copy
	 attributes from the Plotter's GC to it */
      pGC = miNewGC (2, pixels);
      _set_common_mi_attributes (R___(_plotter) (voidptr_t)pGC);
      
      /* libxmi's definition of an elliptic arc aligned with the axes */
      arc.x = xorigin;
      arc.y = yorigin;
      arc.width = squaresize_x;
      arc.height = squaresize_y;
      arc.angle1 = startangle;
      arc.angle2 = anglerange;
      
      if (_plotter->drawstate->fill_type)
	/* not transparent, so fill the arc */
	{
	  /* set fg color in GC (and bg color too) */
	  _plotter->set_fill_color (S___(_plotter));
	  fgPixel.type = MI_PIXEL_INDEX_TYPE;
	  fgPixel.u.index = _plotter->drawstate->i_fill_color_index;
	  pixels[0] = bgPixel;
	  pixels[1] = fgPixel;
	  miSetGCPixels (pGC, 2, pixels);
	  
	  /* fill the arc */
	  miFillArcs ((miPaintedSet *)_plotter->i_painted_set, pGC, 1, &arc);
	}
      
      if (_plotter->drawstate->pen_type)
	/* pen is present, so edge the arc */
	{
	  /* set fg color in GC (and bg color too) */ 
	  _plotter->set_pen_color (S___(_plotter));
	  fgPixel.type = MI_PIXEL_INDEX_TYPE;
	  fgPixel.u.index = _plotter->drawstate->i_pen_color_index;  
	  pixels[0] = bgPixel;
	  pixels[1] = fgPixel;
	  miSetGCPixels (pGC, 2, pixels);
	  
	  /* edge the arc by invoking libxmi's reentrant arc-drawing
	     function, passing it as final argument a pointer to persistent
	     storage maintained by the Plotter */
	  miDrawArcs_r ((miPaintedSet *)_plotter->i_painted_set, pGC, 1, &arc,
			(miEllipseCache *)(_plotter->i_arc_cache_data));
	}
      
      /* deallocate miGC */
      miDeleteGC (pGC);
	  
      /* copy from painted set to canvas, and clear */
      offset.x = 0;
      offset.y = 0;
      miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->i_painted_set, 
				(miCanvas *)_plotter->i_canvas, 
				offset);
      miClearPaintedSet ((miPaintedSet *)_plotter->i_painted_set);
    }

  return;
}
