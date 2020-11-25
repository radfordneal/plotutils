/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for GIF Plotters.  Internally, a GIF Plotter represents
   a path as a polyline.  This version calls the MI scan conversion module
   to edge, and if necessary, fill the polyline. */

#include "sys-defines.h"
#include "extern.h"
#include "g_mi.h"		/* use MI scan conversion module */

/* forward references */
bool _same_index ____P((miPixel pixel1, miPixel pixel2));

int
#ifdef _HAVE_PROTOS
_i_endpath (void)
#else
_i_endpath ()
#endif
{
  int i;
  miIntPoint *miPoints;
  miGC miData;
      
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
  miData.samePixel = _same_index; /* pixel comparison function, see below */
  miData.drawable = _plotter->i_bitmap;
  miData.width = _plotter->i_xn;
  miData.height = _plotter->i_yn;
  miData.bgPixel.index = _plotter->drawstate->i_bg_color_index;

  /* fill in most other attributes, including the dash array */
  _set_common_mi_attributes ((Voidptr)&miData);

  if (!_plotter->drawstate->points_are_connected)
    /* "disconnected" linemode */
    {
      /* set color index, and paint each point */
      _plotter->set_pen_color ();
      miData.fgPixel.index = _plotter->drawstate->i_pen_color_index;
      miPolyPoint (&miData, miCoordModeOrigin,
		  _plotter->drawstate->points_in_path, miPoints);
    }
  else
    /* points are connected by a polyline */
    {
      if (_plotter->drawstate->fill_level)
	{
	  int polygon_type 
	    = (_plotter->drawstate->convex_path ? miConvex : miComplex);

	  /* set color index, and fill the polyline */
	  _plotter->set_fill_color ();
	  miData.fgPixel.index = _plotter->drawstate->i_fill_color_index;
	  miFillPolygon (&miData, polygon_type, miCoordModeOrigin,
			 _plotter->drawstate->points_in_path, miPoints);
	}
      
      /* set color index, and edge the polyline */
      _plotter->set_pen_color ();
      miData.fgPixel.index = _plotter->drawstate->i_pen_color_index;
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
  
  /* something was drawn in frame */
  _plotter->i_frame_nonempty = true;

  return 0;
}

/* Function for comparing pixel values, passed to the MI scan conversion
   module. */
bool
#ifdef _HAVE_PROTOS
_same_index (miPixel pixel1, miPixel pixel2)
#else
_same_index (pixel1, pixel2)
     miPixel pixel1, pixel2;
#endif
{
  return (pixel1.index == pixel2.index ? true : false);
}
