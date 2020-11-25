/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for PNM Plotters.  Internally, a PNM Plotter represents
   a path as a polyline.  This version calls the libxmi scan conversion
   module to edge, and if necessary, fill the polyline. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"		/* use libxmi scan conversion module */

int
#ifdef _HAVE_PROTOS
_n_endpath (S___(Plotter *_plotter))
#else
_n_endpath (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;
  bool closed;
      
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path == 0)	/* nothing to do */
    return 0;
  
  if (_plotter->drawstate->points_in_path == 1)	/* shouldn't happen */
    {
      /* just reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      return 0;
    }
  
  if ((_plotter->drawstate->points_in_path >= 3) /* check for closure */
      && (_plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].x == _plotter->drawstate->datapoints[0].x)
      && (_plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].y == _plotter->drawstate->datapoints[0].y))
    closed = true;
  else
    closed = false;		/* 2-point ones should be open */
  
  /* Special case: disconnected points, no real polyline.  We switch to a
     temporary datapoints buffer for this.  This is a hack, needed because
     the fcircle() method calls endpath(), which would otherwise mess the
     real databuffer up. */
  if (!_plotter->drawstate->points_are_connected)
    {
      plPoint saved_pos;
      plGeneralizedPoint *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_points_in_path = _plotter->drawstate->points_in_path;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _plotter->savestate (S___(_plotter));
      _plotter->pentype (R___(_plotter) 1);
      _plotter->fillcolor (R___(_plotter)
			   _plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (R___(_plotter) 1);
      _plotter->linewidth (R___(_plotter) 0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_points_in_path - (closed ? 1 : 0); i++)
	/* draw each point as a filled circle, diameter = line width */
	_plotter->fcircle (R___(_plotter)
			   saved_datapoints[i].x, saved_datapoints[i].y, 
			   radius);
      _plotter->drawstate->points_are_connected = false;
      _plotter->restorestate (S___(_plotter));
      free (saved_datapoints);
      if (closed)
	_plotter->drawstate->pos = saved_pos; /* restore graphics cursor */
      return 0;
    }
  
  /* general case: points are vertices of a (non-disconnected) polyline */

  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* have something to draw */
    {
      int polyline_len;
      bool identical_user_coordinates = true;
      double xu_last = 0.0, yu_last = 0.0;
      miGC *pGC;
      miPoint *miPoints, offset;
      miPixel fgPixel, bgPixel;
      miPixel pixels[2];
      unsigned char red, green, blue;

      /* construct point array for libxmi module; convert vertices to device
	 coordinates, removing runs */
      miPoints = (miPoint *)_plot_xmalloc (_plotter->drawstate->points_in_path * sizeof(miPoint));
      polyline_len = 0;
      for (i = 0; i < _plotter->drawstate->points_in_path; i++)
	{
	  double xu, yu;
	  int device_x, device_y;
	  
	  xu = _plotter->drawstate->datapoints[i].x;
	  yu = _plotter->drawstate->datapoints[i].y;
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
      _set_common_mi_attributes (R___(_plotter) (voidptr_t)pGC);
      
      if (_plotter->drawstate->fill_type)
	/* not transparent, will fill */
	{
	  double red_d, green_d, blue_d;
	  double desaturate;
	  miPolygonShape polygon_shape
	    = (_plotter->drawstate->convex_path ? MI_SHAPE_CONVEX : MI_SHAPE_GENERAL);
	  
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
	  
	  /* fill the polygon */
	  miFillPolygon ((miPaintedSet *)_plotter->n_painted_set, pGC, 
			 polygon_shape,
			 MI_COORD_MODE_ORIGIN, polyline_len, miPoints);
	}
      
      if (_plotter->drawstate->pen_type)
	/* pen is present, so edge the polyline */
	{
	  /* set pen color in GC to a 24-bit version of our pen color (and
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
	  
	  if (polyline_len > 1)
	    /* have a nondegenerate polyline in integer device space */
	    miDrawLines ((miPaintedSet *)_plotter->n_painted_set, pGC, 
			 MI_COORD_MODE_ORIGIN, polyline_len, miPoints);
	  else
	    /* All user-space points in the polyline were mapped to a
	       single pixel.  If (1) they weren't all the same to begin
	       with, or (2) they were all the same to begin with and the
	       cap mode is "round", then draw as a filled circle of
	       diameter equal to the line width; otherwise draw nothing. */
	    if (identical_user_coordinates == false
		|| _plotter->drawstate->cap_type == CAP_ROUND)
	      {
		unsigned int sp_size 
		  = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
		if (sp_size == 0) 
		  sp_size = 1;
		
		if (sp_size == 1)	/* why why oh why? */
		  miDrawPoints ((miPaintedSet *)_plotter->n_painted_set, pGC, 
				MI_COORD_MODE_ORIGIN, 1, miPoints);
		else
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
		    miFillArcs ((miPaintedSet *)_plotter->n_painted_set, pGC, 1, &arc);
		  }
	      }
	}
  
      /* deallocate miGC and free temporary points array */
      miDeleteGC (pGC);
      free (miPoints);
      
      /* copy from painted set to canvas, and clear */
      offset.x = 0;
      offset.y = 0;
      miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->n_painted_set, 
				(miCanvas *)_plotter->n_canvas, 
				offset);
      miClearPaintedSet ((miPaintedSet *)_plotter->n_painted_set);
    }

  /* free current path */
  _plotter->drawstate->points_in_path = 0;
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;

  return 0;
}
