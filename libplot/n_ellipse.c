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
_n_fellipse (R___(Plotter *_plotter) double xc, double yc, double rx, double ry, double angle)
#else
_n_fellipse (R___(_plotter) xc, yc, rx, ry, angle)
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
  _n_draw_elliptic_mi_arc_internal (R___(_plotter) 
				    xorigin, yorigin, 
				    squaresize_x, squaresize_y, 
				    0, 64 * 360);
  
  _plotter->drawstate->pos.x = xc; /* move to center (a libplot convention) */
  _plotter->drawstate->pos.y = yc;
  
  return 0;
}

/* Draw an elliptic arc aligned with the coordinate axes, by invoking
   functions in the libxmi API.  Takes account of the possible need for
   filling.

   Only the case squaresize_x > 0 and squaresize_y > 0 is handled, since
   miFillArcs() and miDrawArcs() do not support squaresize_x = 0 or
   squaresize_y = 0. */

void
#ifdef _HAVE_PROTOS
_n_draw_elliptic_mi_arc_internal (R___(Plotter *_plotter) int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
#else
_n_draw_elliptic_mi_arc_internal (R___(_plotter) xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange)
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
      unsigned char red, green, blue;

      /* determine background pixel color */
      bgPixel.type = MI_PIXEL_RGB_TYPE;
      bgPixel.u.rgb[0] = _plotter->drawstate->bgcolor.red & 0xff;
      bgPixel.u.rgb[1] = _plotter->drawstate->bgcolor.green & 0xff;
      bgPixel.u.rgb[2] = _plotter->drawstate->bgcolor.blue & 0xff;
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
	  double red_d, green_d, blue_d;
	  double desaturate;
	  
	  /* scale fillcolor RGB values from 16-bits to range [0.0,1.0] */
	  red_d = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
	  green_d = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
	  blue_d = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;
	  
	  /* fill_type, if nonzero, specifies the extent to which the
	     nominal fill color should be desaturated.  1 means no
	     desaturation, 0xffff means complete desaturation (white). */
	  desaturate = ((double)_plotter->drawstate->fill_type - 1.)/0xFFFE;
	  red_d = red_d + desaturate * (1.0 - red_d);
	  green_d = green_d + desaturate * (1.0 - green_d);
	  blue_d = blue_d + desaturate * (1.0 - blue_d);
	  
	  /* convert desaturated RGB values to 8 bits each (24 bits in all) */
	  red = IROUND(0xff * red_d);
	  green = IROUND(0xff * green_d);
	  blue = IROUND(0xff * blue_d);
	  
	  /* set fg color in GC to the 24-bit desaturated RGB (and set bg
	     color too) */
	  fgPixel.type = MI_PIXEL_RGB_TYPE;
	  fgPixel.u.rgb[0] = red;
	  fgPixel.u.rgb[1] = green;
	  fgPixel.u.rgb[2] = blue;
	  pixels[0] = bgPixel;
	  pixels[1] = fgPixel;
	  miSetGCPixels (pGC, 2, pixels);
	  
	  /* fill the arc */
	  miFillArcs ((miPaintedSet *)_plotter->n_painted_set, pGC, 1, &arc);
	}

      if (_plotter->drawstate->pen_type)
	/* pen is present, so edge the arc */
	{
	  /* set fg color in GC to a 24-bit version of our pen color (and
	     set bg color too) */
	  red = ((unsigned int)(_plotter->drawstate->fgcolor.red) >> 8) & 0xff;
	  green = ((unsigned int)(_plotter->drawstate->fgcolor.green) >> 8) & 0xff;
	  blue = ((unsigned int)(_plotter->drawstate->fgcolor.blue) >> 8) & 0xff;  
	  fgPixel.type = MI_PIXEL_RGB_TYPE;
	  fgPixel.u.rgb[0] = red;
	  fgPixel.u.rgb[1] = green;
	  fgPixel.u.rgb[2] = blue;
	  pixels[0] = bgPixel;
	  pixels[1] = fgPixel;
	  miSetGCPixels (pGC, 2, pixels);
	  
	  /* edge the arc by invoking libxmi's reentrant arc-drawing
	     function, passing it as final argument a pointer to persistent
	     storage maintained by the Plotter */
	  miDrawArcs_r ((miPaintedSet *)_plotter->n_painted_set, pGC, 1, &arc,
			(miEllipseCache *)(_plotter->n_arc_cache_data));
	  
	}
      
      /* deallocate miGC */
      miDeleteGC (pGC);
      
      /* copy from painted set to canvas, and clear */
      offset.x = 0;
      offset.y = 0;
      miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->n_painted_set, 
				(miCanvas *)_plotter->n_canvas, 
				offset);
      miClearPaintedSet ((miPaintedSet *)_plotter->n_painted_set);
    }

  return;
}
