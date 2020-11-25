/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for PNM Plotters.  Internally, a PNM Plotter represents
   a path as a polyline.  This version calls the MI scan conversion module
   to edge, and if necessary, fill the polyline. */

#include "sys-defines.h"
#include "extern.h"
#include "g_mi.h"		/* use MI scan conversion module */

/* forward references */
bool _same_rgb ____P((miPixel pixel1, miPixel pixel2));

int
#ifdef _HAVE_PROTOS
_n_endpath (void)
#else
_n_endpath ()
#endif
{
  int i;
  miIntPoint *miPoints;
  miGC miData;
  unsigned char red, green, blue;
      
  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
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
  
  /* construct point array for MI module */
  miPoints = (miIntPoint *)_plot_xmalloc (_plotter->drawstate->points_in_path * sizeof(miIntPoint));
  for (i = 0; i < _plotter->drawstate->points_in_path; i++)
    {
      miPoints[i].x = IROUND(XD(_plotter->drawstate->datapoints[i].x, 
				_plotter->drawstate->datapoints[i].y));
      miPoints[i].y = IROUND(YD(_plotter->drawstate->datapoints[i].x, 
				_plotter->drawstate->datapoints[i].y));
    }

  /* construct miGC ("augmented graphics context", i.e., graphics context
     plus drawable) for the MI module */
  miData.samePixel = _same_rgb; /* pixel comparison function, see below */
  miData.drawable = _plotter->n_bitmap;
  miData.width = _plotter->n_xn;
  miData.height = _plotter->n_yn;
  miData.bgPixel.rgb[0] = _plotter->drawstate->bgcolor.red & 0xff;
  miData.bgPixel.rgb[1] = _plotter->drawstate->bgcolor.green & 0xff;
  miData.bgPixel.rgb[2] = _plotter->drawstate->bgcolor.blue & 0xff;

  /* fill in most other attributes, including the dash buffer */
  _set_common_mi_attributes ((Voidptr)&miData);

  if (!_plotter->drawstate->points_are_connected)
    /* "disconnected" linemode */
    {
      /* set MI's pen color to be a 24-bit version of our pen color, and
         paint each point */
      red = ((unsigned int)(_plotter->drawstate->fgcolor.red) >> 8) & 0xff;
      green = ((unsigned int)(_plotter->drawstate->fgcolor.green) >> 8) & 0xff;
      blue = ((unsigned int)(_plotter->drawstate->fgcolor.blue) >> 8) & 0xff;  
      miData.fgPixel.rgb[0] = red;
      miData.fgPixel.rgb[1] = green;
      miData.fgPixel.rgb[2] = blue;
      miPolyPoint (&miData, miCoordModeOrigin,
		  _plotter->drawstate->points_in_path, miPoints);
    }
  else
    /* points are connected by a polyline */
    {
      if (_plotter->drawstate->fill_level)
	{
	  double red_d, green_d, blue_d;
	  double desaturate;
	  int polygon_type 
	    = (_plotter->drawstate->convex_path ? miConvex : miComplex);

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
             polyline */
	  miData.fgPixel.rgb[0] = red;
	  miData.fgPixel.rgb[1] = green;
	  miData.fgPixel.rgb[2] = blue;
	  miFillPolygon (&miData, polygon_type, miCoordModeOrigin,
			 _plotter->drawstate->points_in_path, miPoints);
	}
      
      /* set MI's pen color to a 24-bit version of our pen color, and edge
	 the polyline */
      red = ((unsigned int)(_plotter->drawstate->fgcolor.red) >> 8) & 0xff;
      green = ((unsigned int)(_plotter->drawstate->fgcolor.green) >> 8) & 0xff;
      blue = ((unsigned int)(_plotter->drawstate->fgcolor.blue) >> 8) & 0xff;  
      miData.fgPixel.rgb[0] = red;
      miData.fgPixel.rgb[1] = green;
      miData.fgPixel.rgb[2] = blue;
      if (miData.lineStyle == miLineSolid)
	miWideLine (&miData, miCoordModeOrigin,
		    _plotter->drawstate->points_in_path, miPoints);
      else
	miWideDash (&miData, miCoordModeOrigin,
		    _plotter->drawstate->points_in_path, miPoints);
    }
  
  /* free dash array (created by _set_common_mi_attributes() above) */
  free (miData.dash);

  /* free everything */
  free (miPoints);
  _plotter->drawstate->points_in_path = 0;
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  
  return 0;
}

/* Function for comparing pixel values, passed to the MI scan conversion
   module. */
bool
#ifdef _HAVE_PROTOS
_same_rgb (miPixel pixel1, miPixel pixel2)
#else
_same_rgb (pixel1, pixel2)
     miPixel pixel1, pixel2;
#endif
{
  return ((pixel1.rgb[0] == pixel2.rgb[0]
	   && pixel1.rgb[1] == pixel2.rgb[1]
	   && pixel1.rgb[2] == pixel2.rgb[2]) ? true : false);
}
