/* This file contains the internal paint_path() method, which the public
   method endpath() is a wrapper around. */

/* This version is for Bitmap Plotters.  By construction, for Bitmap
   Plotters our path buffer always contains either a segment list, or an
   ellipse object.  If it's a segment list, it contains either (1) a
   sequence of line segments, or (2) a single circular or elliptic arc
   segment.  Those are all sorts of path that libxmi can handle.  (For an
   ellipse or circular/elliptic arc segment to have been added to the path
   buffer, the map from user to device coordinates must preserve axes.) */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"		/* use libxmi scan conversion module */

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

void
#ifdef _HAVE_PROTOS
_b_paint_path (S___(Plotter *_plotter))
#else
_b_paint_path (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	int i, polyline_len;
	bool identical_user_coordinates = true;
	double xu_last, yu_last;
	miGC *pGC;
	miPoint *miPoints, offset;
	miPixel fgPixel, bgPixel;
	miPixel pixels[2];
	unsigned char red, green, blue;
	plPoint p0, p1, pc;

	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1)/* shouldn't happen */
	  break;

	if (_plotter->drawstate->path->num_segments == 2
	    && _plotter->drawstate->path->segments[1].type == S_ARC)
	  /* segment buffer contains a single circular arc, not a polyline */
	  {
	    p0 = _plotter->drawstate->path->segments[0].p;
	    p1 = _plotter->drawstate->path->segments[1].p;
	    pc = _plotter->drawstate->path->segments[1].pc;
	    
	    /* use libxmi rendering */
	    _b_draw_elliptic_arc (R___(_plotter) p0, p1, pc);

	    break;
	  }

	if (_plotter->drawstate->path->num_segments == 2
	    && _plotter->drawstate->path->segments[1].type == S_ELLARC)
	  /* segment buffer contains a single elliptic arc, not a polyline */
	  {
	    p0 = _plotter->drawstate->path->segments[0].p;
	    p1 = _plotter->drawstate->path->segments[1].p;
	    pc = _plotter->drawstate->path->segments[1].pc;
	    
	    /* use libxmi rendering */
	    _b_draw_elliptic_arc_2 (R___(_plotter) p0, p1, pc);
	    
	    break;
	  }

	/* neither of above applied, so segment buffer contains a polyline,
	   not an arc */

	/* construct point array for libxmi module; convert vertices to
	   device coordinates, removing runs */
	miPoints = (miPoint *)_plot_xmalloc (_plotter->drawstate->path->num_segments * sizeof(miPoint));

	polyline_len = 0;
	xu_last = 0.0;
	yu_last = 0.0;
	identical_user_coordinates = true;
	for (i = 0; i < _plotter->drawstate->path->num_segments; i++)
	  {
	    double xu, yu;
	    int device_x, device_y;
	    
	    xu = _plotter->drawstate->path->segments[i].p.x;
	    yu = _plotter->drawstate->path->segments[i].p.y;
	    if (i > 0 && (xu != xu_last || yu != yu_last))
	      /* in user space, not all points are the same */
	      identical_user_coordinates = false;	
	    device_x = IROUND(XD(xu, yu));
	    device_y = IROUND(YD(xu, yu));
	    if ((polyline_len == 0) 
		|| (device_x != miPoints[polyline_len-1].x) 
		|| (device_y != miPoints[polyline_len-1].y))
	      /* add point, in integer device coordinates, to the array */
	      {
		miPoints[polyline_len].x = device_x;
		miPoints[polyline_len].y = device_y;
		polyline_len++;
	      }
	    xu_last = xu;
	    yu_last = yu;
	  }
	
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
	_set_common_mi_attributes (_plotter->drawstate, (voidptr_t)pGC);
	
	if (_plotter->drawstate->fill_type)
	  /* not transparent, will fill */
	  {
	    /* flattened drawing primitives, i.e., box/circle/ellipse,
	       are always convex */
	    miPolygonShape polygon_shape
	      = (_plotter->drawstate->path->primitive ? MI_SHAPE_CONVEX : MI_SHAPE_GENERAL);
	    
	    /* set fg color in GC to a 24-bit version of our fill color */
	    red = ((unsigned int)(_plotter->drawstate->fillcolor.red) >> 8) & 0xff;
	    green = ((unsigned int)(_plotter->drawstate->fillcolor.green) >> 8) & 0xff;
	    blue = ((unsigned int)(_plotter->drawstate->fillcolor.blue) >> 8) & 0xff;  
	    fgPixel.type = MI_PIXEL_RGB_TYPE;
	    fgPixel.u.rgb[0] = red;
	    fgPixel.u.rgb[1] = green;
	    fgPixel.u.rgb[2] = blue;
	    pixels[0] = bgPixel;
	    pixels[1] = fgPixel;
	    miSetGCPixels (pGC, 2, pixels);
	    
	    /* do the filling */

	    if (_plotter->drawstate->path->num_segments > 1 
		&& polyline_len == 1)
	      /* special case: all user-space points in polyline were
		 mapped to a single integer pixel, so just paint it */
	      miDrawPoints ((miPaintedSet *)_plotter->b_painted_set, pGC, 
			    MI_COORD_MODE_ORIGIN, 1, miPoints);
	    else
	      /* normal case */
	      miFillPolygon ((miPaintedSet *)_plotter->b_painted_set, pGC, 
			     polygon_shape,
			     MI_COORD_MODE_ORIGIN, polyline_len, miPoints);
	  }
	
	if (_plotter->drawstate->pen_type)
	  /* pen is present, so edge the polyline */
	  {
	    /* set pen color in GC to a 24-bit version of our pen color
	       (and set bg color too) */
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
	    
	    if (polyline_len == 1)
	      /* Special case: all user-space points in the polyline were
		 mapped to a single pixel.  If (1) they weren't all the
		 same to begin with, or (2) they were all the same to begin
		 with and the cap mode is "round", then draw as a filled
		 circle of diameter equal to the line width; otherwise draw
		 nothing. */
	      {
		if (identical_user_coordinates == false
		    || _plotter->drawstate->cap_type == CAP_ROUND)
		  {
		    unsigned int sp_size 
		      = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
		    if (sp_size == 0) 
		      sp_size = 1;
		    
		    if (sp_size == 1)
		      /* subcase: just draw a point */
		      miDrawPoints ((miPaintedSet *)_plotter->b_painted_set, pGC, 
				    MI_COORD_MODE_ORIGIN, 1, miPoints);
		    else
		      /* draw a filled circle */
		      {
			int sp_offset;
			miArc arc;
			
			sp_offset = 
			  (_plotter->drawstate->quantized_device_line_width + 1) / 2;
			arc.x = miPoints[0].x - sp_offset;
			arc.y = miPoints[0].y - sp_offset;
			arc.width = sp_size;
			arc.height = sp_size;
			arc.angle1 = 0;
			arc.angle2 = 64 * 360;
			miFillArcs ((miPaintedSet *)_plotter->b_painted_set,
				    pGC, 1, &arc);
		      }
		  }
	      }
	    
	    else
	      /* normal case: draw a nondegenerate polyline in integer
                 device space */
	      miDrawLines ((miPaintedSet *)_plotter->b_painted_set, pGC, 
			   MI_COORD_MODE_ORIGIN, polyline_len, miPoints);
	  }
	
	/* deallocate miGC and free temporary points array */
	miDeleteGC (pGC);
	free (miPoints);
	
	/* copy from painted set to canvas, and clear */
	offset.x = 0;
	offset.y = 0;
	miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->b_painted_set, 
				  (miCanvas *)_plotter->b_canvas, 
				  offset);
	miClearPaintedSet ((miPaintedSet *)_plotter->b_painted_set);
      }
      break;
      
    case (int)PATH_ELLIPSE:
      {
	int ninetymult;
	int x_orientation, y_orientation;
	int xorigin, yorigin;
	unsigned int squaresize_x, squaresize_y;
	plPoint pc;
	double rx, ry, angle;

	pc = _plotter->drawstate->path->pc;
	rx = _plotter->drawstate->path->rx;
	ry = _plotter->drawstate->path->ry;
	angle = _plotter->drawstate->path->angle;	

	/* if angle is multiple of 90 degrees, modify to permit use of
	   libxmi's arc rendering */
	ninetymult = IROUND(angle / 90.0);
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
	
	/* axes flipped? (by default y-axis is, due to libxmi's flipped-y
           convention) */
	x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
	y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);
	
	/* location of `origin' (upper left corner of bounding rect. for
	   ellipse) and width and height; libxmi's flipped-y convention
	   affects these values */
	xorigin = IROUND(XD(pc.x - x_orientation * rx, 
			    pc.y - y_orientation * ry));
	yorigin = IROUND(YD(pc.x - x_orientation * rx, 
			    pc.y - y_orientation * ry));
	squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
	squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
	/* Because this ellipse object was added to the path buffer, we
	   already know that (1) the user->device frame map preserves
	   coordinate axes, (2) effectively, angle == 0.  These are
	   necessary for the libxmi scan-conversion module to do the
	   drawing. */

	/* draw ellipse (elliptic arc aligned with the coordinate axes, arc
	   range = 64*360 64'ths of a degree) */
	_b_draw_elliptic_arc_internal (R___(_plotter) 
					  xorigin, yorigin, 
					  squaresize_x, squaresize_y, 
					  0, 64 * 360);
      }
      break;
      
    default:			/* shouldn't happen */
      break;
    }
}

/* Use libxmi rendering to draw what would be a circular arc in the user
   frame.  If this is called, the map from user to device coordinates is
   assumed to preserve coordinate axes (it may be anisotropic [x and y
   directions scaled differently], and it may include a reflection through
   either or both axes).  So it will be a circular or elliptic arc in the
   device frame, of the sort that libxmi supports. */

void
#ifdef _HAVE_PROTOS
_b_draw_elliptic_arc (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
#else
_b_draw_elliptic_arc (R___(_plotter) p0, p1, pc)
     S___(Plotter *_plotter;)
     plPoint p0, p1, pc;
#endif
{
  double radius;
  double theta0, theta1;
  int startangle, anglerange;
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  /* axes flipped? (by default y-axis is, due to xmi's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  /* radius of circular arc in user frame is distance to p0, and also to p1 */
  radius = DIST(pc, p0);

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; X's flipped-y convention affects these values */
  xorigin = IROUND(XD(pc.x - x_orientation * radius, 
		      pc.y - y_orientation * radius));
  yorigin = IROUND(YD(pc.x - x_orientation * radius, 
		      pc.y - y_orientation * radius));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * radius, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * radius));

  theta0 = _xatan2 (-y_orientation * (p0.y - pc.y), 
		    x_orientation * (p0.x - pc.x)) / M_PI;
  theta1 = _xatan2 (-y_orientation * (p1.y - pc.y), 
		    x_orientation * (p1.x - pc.x)) / M_PI;

  if (theta1 < theta0)
    theta1 += 2.0;		/* adjust so that difference > 0 */
  if (theta0 < 0.0)
    {
      theta0 += 2.0;		/* adjust so that startangle > 0 */
      theta1 += 2.0;
    }

  if (theta1 - theta0 > 1.0)	/* swap if angle appear to be > 180 degrees */
    {
      double tmp;
      
      tmp = theta0;
      theta0 = theta1;
      theta1 = tmp;
      theta1 += 2.0;		/* adjust so that difference > 0 */      
    }

  if (theta0 >= 2.0 && theta1 >= 2.0)
    /* avoid obscure X bug */
    {
      theta0 -= 2.0;
      theta1 -= 2.0;
    }

  startangle = IROUND(64 * theta0 * 180.0); /* in 64'ths of a degree */
  anglerange = IROUND(64 * (theta1 - theta0) * 180.0); /* likewise */

  _b_draw_elliptic_arc_internal (R___(_plotter)
				 xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
}

/* Use libxmi rendering to draw what would be a quarter-ellipse in the user
   frame.  If this is called, the map from user to device coordinates is
   assumed to preserve coordinate axes (it may be anisotropic [x and y
   directions scaled differently], and it may include a reflection through
   either or both axes).  So it will be a quarter-ellipse in the device
   frame, of the sort that libxmi supports. */
void
#ifdef _HAVE_PROTOS
_b_draw_elliptic_arc_2 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
#else
_b_draw_elliptic_arc_2 (R___(_plotter) p0, p1, pc)
     S___(Plotter *_plotter;)
     plPoint p0, p1, pc;
#endif
{
  double rx, ry;
  double x0, y0, x1, y1, xc, yc;
  int startangle, endangle, anglerange;
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  /* axes flipped? (by default y-axis is, due to xmi's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  xc = pc.x, yc = pc.y;
  x0 = p0.x, y0 = p0.y;
  x1 = p1.x, y1 = p1.y;

  if (y0 == yc && x1 == xc)
    /* initial pt. on x-axis, final pt. on y-axis */
    {
      /* semi-axes in user frame */
      rx = (x0 > xc) ? x0 - xc : xc - x0;
      ry = (y1 > yc) ? y1 - yc : yc - y1;
      /* starting and ending angles; note flipped-y convention */
      startangle = ((x0 > xc ? 1 : -1) * x_orientation == 1) ? 0 : 180;
      endangle = ((y1 > yc ? 1 : -1) * y_orientation == -1) ? 90 : 270;
    }
  else
    /* initial pt. on y-axis, final pt. on x-axis */
    {	
      /* semi-axes in user frame */
      rx = (x1 > xc) ? x1 - xc : xc - x1;
      ry = (y0 > yc) ? y0 - yc : yc - y0;
      /* starting and ending angles; note flipped-y convention */
      startangle = ((y0 > yc ? 1 : -1) * y_orientation == -1) ? 90 : 270;
      endangle = ((x1 > xc ? 1 : -1) * x_orientation == 1) ? 0 : 180;
    }	  

  if (endangle < startangle)
    endangle += 360;
  anglerange = endangle - startangle; /* always 90 or 270 */

  /* our convention: a quarter-ellipse can only be 90 degrees
     of a libxmi ellipse, not 270 degrees, so interchange points */
  if (anglerange == 270)
    {
      int tmp;

      tmp = startangle;
      startangle = endangle;
      endangle = tmp;
      anglerange = 90;
    }
      
  if (startangle >= 360)
    /* avoid obscure libxmi bug */
    startangle -= 360;		/* endangle no longer relevant */

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; xmi's flipped-y convention affects these values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));
      
  /* reexpress in 64'ths of a degree (libxmi convention) */
  startangle *= 64;
  anglerange *= 64;

  _b_draw_elliptic_arc_internal (R___(_plotter)
				 xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
}

/* Draw an elliptic arc aligned with the coordinate axes, by invoking
   functions in the libxmi API.  Takes account of the possible need for
   filling.

   The cases squaresize_x = 0 and squaresize_y = 0 are handled specially,
   since miFillArcs() and miDrawArcs() do not support them. */

void
#ifdef _HAVE_PROTOS
_b_draw_elliptic_arc_internal (R___(Plotter *_plotter) int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
#else
_b_draw_elliptic_arc_internal (R___(_plotter) xorigin, yorigin, squaresize_x, squaresize_y, startangle, anglerange)
     S___(Plotter *_plotter;)
     int xorigin, yorigin; 
     unsigned int squaresize_x, squaresize_y; 
     int startangle, anglerange;
#endif
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
  _set_common_mi_attributes (_plotter->drawstate, (voidptr_t)pGC);
  
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
      
      /* set fg color in GC to the 24-bit desaturated RGB (and set bg color
	 too) */
      fgPixel.type = MI_PIXEL_RGB_TYPE;
      fgPixel.u.rgb[0] = red;
      fgPixel.u.rgb[1] = green;
      fgPixel.u.rgb[2] = blue;
      pixels[0] = bgPixel;
      pixels[1] = fgPixel;
      miSetGCPixels (pGC, 2, pixels);
      
      /* fill the arc */
      if (squaresize_x <= 1 || squaresize_y <= 1)
	/* a special case, which miFillArcs() doesn't handle in the way we'd
	   like; just paint a single pixel, irrespective of angle range */
	{
	  miPoint point;

	  point.x = xorigin;
	  point.y = yorigin;
	  miDrawPoints ((miPaintedSet *)_plotter->b_painted_set, 
			pGC, MI_COORD_MODE_ORIGIN, 1, &point);
	}
      else
	/* default case */
	miFillArcs ((miPaintedSet *)_plotter->b_painted_set, pGC, 1, &arc);
    }

  if (_plotter->drawstate->pen_type)
    /* pen is present, so edge the arc */
    {
      unsigned int sp_size = 0;	/* keep compiler happy */

      /* set fg color in GC to a 24-bit version of our pen color (and set
	 bg color too) */
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
      
      if (squaresize_x <= 1 || squaresize_y <= 1)
	/* Won't call miDrawArcs in the usual way, because it performs
           poorly when one of these two is zero, at least.  Irrespective of
           angle range, will fill a disk of diameter equal to line width */
	{
	  int sp_offset;

	  sp_size 
	    = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
	  sp_offset
	    = (int)(_plotter->drawstate->quantized_device_line_width + 1) / 2;
	  
	  if (sp_size == 0) 
	    sp_size = 1;
	  arc.x -= sp_offset;
	  arc.y -= sp_offset;	  
	  arc.width = sp_size;
	  arc.height = sp_size;
	  arc.angle1 = 0;
	  arc.angle2 = 64 * 360;
	}

      /* edge the arc by invoking libxmi's reentrant arc-drawing function,
	 passing it as final argument a pointer to persistent storage
	 maintained by the Plotter */

      if (squaresize_x <= 1 || squaresize_y <= 1)
	/* miDrawArcs doesn't handle this case as we'd wish, will
	   treat specially */
	{
	  if (sp_size == 1)
	    /* special subcase: line width is small too, so just paint a
	       single pixel rather than filling abovementioned disk */
	    {
	      miPoint point;
	      
	      point.x = xorigin;
	      point.y = yorigin;
	      miDrawPoints ((miPaintedSet *)_plotter->b_painted_set, 
			    pGC, MI_COORD_MODE_ORIGIN, 1, &point);
	    }
	  else
	    /* normal version of special case: draw filled disk of diameter
	       equal to the line width, irrespective of the angle range */
	    miFillArcs((miPaintedSet *)_plotter->b_painted_set, pGC, 1, &arc);
	}
      else
	/* default case, which is what is almost always used */
	miDrawArcs_r ((miPaintedSet *)_plotter->b_painted_set, pGC, 1, &arc,
		      (miEllipseCache *)(_plotter->b_arc_cache_data));
    }
      
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

bool
#ifdef _HAVE_PROTOS
_b_paint_paths (S___(Plotter *_plotter))
#else
_b_paint_paths (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return false;
}
