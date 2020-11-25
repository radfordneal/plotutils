/* This file contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

#include "sys-defines.h"
#include "extern.h"
#include "g_mi.h"		/* use MI scan conversion module */

/* In this version, we first check to see if the angle of inclination is
   zero, and the map from the user frame to the device frame preserves
   axes.  If these are both true, we can use the MI module to draw the
   ellipse.  If they are not, we draw an inscribed polyline, by invoking
   the generic method. */

int
#ifdef _HAVE_PROTOS
_n_fellipse (double xc, double yc, double rx, double ry, double angle)
#else
_n_fellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
#endif
{
  int ninetymult = IROUND(angle / 90.0);
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
      /* just move to center (a libplot convention), flushing path if any */
      _plotter->fmove (xc, yc);
      return 0;
    }
  
  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath ();	/* flush path if any */

 /* if angle is multiple of 90 degrees, modify to permit use of native X
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

  /* If ellipse must be drawn at an angle, or affine map from user frame to
     device frame does not preserve coordinate axes, call the generic class
     method to draw an inscribed polyline.  Specify that it's convex (to
     speed up rendering, if we're filling). */

  if (angle != 0.0 || !(_plotter->drawstate->transform.axes_preserved))
    {
      int retval;

      /* draw, move to center, flush out */
      retval = _g_fellipse (xc, yc, rx, ry, angle); 
      return retval;
    }

  /* otherwise use MI scan-conversion module, as follows */

  rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure MI problems */
  ry = (ry < 0.0 ? -ry : ry);  

  /* axes flipped? (by default y-axis is, due to MI's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);
  
  /* location of `origin' (upper left corner of bounding rect. on
     display) and width and height; MI's flipped-y convention affects
     these values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
  
  /* don't use zero dimensions if user specified nonzero */
  if (squaresize_x == 0 && rx > 0.0)
    squaresize_x = 1;
  if (squaresize_y == 0 && ry > 0.0)
    squaresize_y = 1;
  
  /* draw ellipse (elliptic arc, arc range = 64*360 64'ths of a degree) */
  _n_draw_elliptic_mi_arc_internal (xorigin, yorigin, 
				    squaresize_x, squaresize_y, 
				    0, 64 * 360);
  
  _plotter->drawstate->pos.x = xc; /* move to center (a libplot convention) */
  _plotter->drawstate->pos.y = yc;
  
  return 0;
}

void
#ifdef _HAVE_PROTOS
_n_draw_elliptic_mi_arc_internal (int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
#else
_n_draw_elliptic_mi_arc_internal (xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange)
     int xorigin, yorigin; 
     unsigned int squaresize_x, squaresize_y; 
     int startangle, anglerange;
#endif
{
  miGC miData;
  miArc arc;
  unsigned char red, green, blue;

  /* should check for width, height being 0 here, and treat this special
     case appropriately?  FIXME. */

  /* place current line attributes in GC's used for drawing and filling */
  _plotter->set_attributes();  

  /* construct miGC ("augmented graphics context", i.e., graphics context
     plus drawable) for the MI module */
  miData.drawable = _plotter->n_bitmap;
  miData.width = _plotter->n_xn;
  miData.height = _plotter->n_yn;
  miData.bgPixel.rgb[0] = _plotter->drawstate->bgcolor.red & 0xff;
  miData.bgPixel.rgb[1] = _plotter->drawstate->bgcolor.green & 0xff;
  miData.bgPixel.rgb[2] = _plotter->drawstate->bgcolor.blue & 0xff;

  /* fill in most other attributes, including the dash array */
  _set_common_mi_attributes ((Voidptr)&miData);

  /* MI's definition of an elliptic arc aligned with the axes */
  arc.x = xorigin;
  arc.y = yorigin;
  arc.width = squaresize_x;
  arc.height = squaresize_y;
  arc.angle1 = startangle;
  arc.angle2 = anglerange;

  if (_plotter->drawstate->fill_level) /* not transparent */
    {
      double red_d, green_d, blue_d;
      double desaturate;
      
      /* scale fillcolor RGB values from 16-bits to range [0.0,1.0] */
      red_d = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
      green_d = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
      blue_d = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;
      
      /* fill_level, if nonzero, specifies the extent to which the
	 nominal fill color should be desaturated.  1 means no
	 desaturation, 0xffff means complete desaturation (white). */
      desaturate = ((double)_plotter->drawstate->fill_level - 1.)/0xFFFE;
      red_d = red_d + desaturate * (1.0 - red_d);
      green_d = green_d + desaturate * (1.0 - green_d);
      blue_d = blue_d + desaturate * (1.0 - blue_d);
      
      /* convert desaturated RGB values to 8 bits each (24 bits in all) */
      red = IROUND(0xff * red_d);
      green = IROUND(0xff * green_d);
      blue = IROUND(0xff * blue_d);
      
      /* set MI's pen color to the 24-bit desaturated RGB, and fill the
	 arc */
      miData.fgPixel.rgb[0] = red;
      miData.fgPixel.rgb[1] = green;
      miData.fgPixel.rgb[2] = blue;
      
      miPolyFillArc (&miData, 1, &arc);
    }

  /* set MI's pen color to a 24-bit version of our pen color, and edge
     the arc */
  red = ((unsigned int)(_plotter->drawstate->fgcolor.red) >> 8) & 0xff;
  green = ((unsigned int)(_plotter->drawstate->fgcolor.green) >> 8) & 0xff;
  blue = ((unsigned int)(_plotter->drawstate->fgcolor.blue) >> 8) & 0xff;  
  miData.fgPixel.rgb[0] = red;
  miData.fgPixel.rgb[1] = green;
  miData.fgPixel.rgb[2] = blue;
  
  if (miData.lineWidth > 0)
    miPolyArc (&miData, 1, &arc);
  else
    miZeroPolyArc (&miData, 1, &arc);

  /* free dash array (created by _set_common_mi_attributes() above) */
  free (miData.dash);

  return;
}
