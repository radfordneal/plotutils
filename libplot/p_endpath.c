/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for PSPlotters.  By construction, for PSPlotters our
   path storage buffer includes a sequence of line segments (no other path
   elements such as arcs). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* 16-bit brush patterns for idraw (1 = on, 0 = off), indexed by line type */
const long _idraw_brush_pattern[NUM_LINE_TYPES] = 
/* solid, dotted, dotdashed, shortdashed, longdashed */
{ 0xffff, 0x8888, 0xc3f0, 0xf0f0, 0xffc0 };

/* Corresponding dash arrays for PS (cylically used, on/off/on/off...).
   PS_DASH_ARRAY_LEN is defined in extern.h as 4; do not change it without
   checking the code below (it depends on this value). */
const char _ps_dash_array[NUM_LINE_TYPES][PS_DASH_ARRAY_LEN] =
/* these on/off bit patterns are those used by our X11 driver, and also by
   the xterm Tektronix emulator, except that the emulator seems incorrectly
   to have on and off interchanged */
{
  {0, 0, 0, 0},			/* solid (dummy) */
  {1, 3, 1, 3},			/* dotted */
  {1, 3, 4, 3},			/* dotdashed */
  {4, 4, 4, 4},			/* shortdashed */
  {7, 4, 7, 4}			/* longdashed */
};

/* PS join styles, indexed by internal join type number (miter/rd./bevel) */
const int _ps_join_style[] =
{ PS_JOIN_MITER, PS_JOIN_ROUND, PS_JOIN_BEVEL };

/* PS cap styles, indexed by internal cap type number (butt/rd./project) */
const int _ps_cap_style[] =
{ PS_CAP_BUTT, PS_CAP_ROUND, PS_CAP_PROJECT };

int
#ifdef _HAVE_PROTOS
_p_endpath (void)
#else
_p_endpath ()
#endif
{
  int i, numpoints;
  bool closed, singular_map;
  double linewidth, invnorm = 0.0, granularity = 1.0;
  double dashbuf[PS_DASH_ARRAY_LEN], scale;

  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path == 0)	/* nothing to do */
    return 0;
  if (_plotter->drawstate->points_in_path == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
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
      Point saved_pos;
      GeneralizedPoint *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_points_in_path = _plotter->drawstate->points_in_path;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _plotter->savestate();
      _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (1);
      _plotter->linewidth (0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_points_in_path - (closed ? 1 : 0); i++)
	/* draw each point as a filled circle, diameter = line width */
	_plotter->fcircle (saved_datapoints[i].x, saved_datapoints[i].y, 
			   radius);
      _plotter->drawstate->points_are_connected = false;

      _plotter->restorestate();
      free (saved_datapoints);
      if (closed)
	_plotter->drawstate->pos = saved_pos; /* restore graphics cursor */
      return 0;
    }
  
  /* general case: points are vertices of a polyline */

  /* compute reciprocal norm of user->device affine transformation; need
     this because transformation matrices we'll emit must be normalized */
  {
    double det, norm;
    
    det = _plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3]
      - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2];
    norm = sqrt(fabs(det));
    /* granularity = scaleup factor for user coordinates, so that when
       they're emitted as integers, resolution loss won't be excessive.
       CTM entries will be scaled down by this factor. */
    granularity = norm / (PS_MIN_RESOLUTION);
    if (norm != 0.0)
      {
	/* invnorm is `norm' of device->user coordinate transformation */
	invnorm = 1.0 / norm;
	singular_map = false;
      }
    else
      singular_map = true;
  }

  /* includes idraw instruction: start of MLine or Poly */
  if (closed)
    strcpy (_plotter->page->point, "\
Begin %I Poly\n");
  else
    strcpy (_plotter->page->point, "\
Begin %I MLine\n");
  _update_buffer (_plotter->page);

  /* redefine `originalCTM' matrix, which is the CTM applied when the
     polyline is stroked (as opposed to drawn).  We define it to be the
     same as the one in effect when the polyline was drawn. */
  if (singular_map != true)
    {
      int integer_linewidth = _plotter->drawstate->quantized_device_line_width;
      double double_linewidth = _plotter->drawstate->device_line_width;
      double linewidth_adjust;

      /* adjustment needed, due to our specifying line widths as integers */
      if (integer_linewidth != 0)
	linewidth_adjust = double_linewidth / integer_linewidth;
      else
	linewidth_adjust = 1.0;

      strcpy (_plotter->page->point, "\
[");
      _update_buffer (_plotter->page);

      for (i = 0; i < 4; i++)
	{
	  sprintf (_plotter->page->point, "%.7g ", 
		   linewidth_adjust * invnorm * _plotter->drawstate->transform.m[i]);
	  _update_buffer (_plotter->page);
	}
      _update_buffer (_plotter->page);
      strcpy (_plotter->page->point, "\
0 0 ] trueoriginalCTM originalCTM\n\
concatmatrix pop\n");
      _update_buffer (_plotter->page);
    }

  /* specify cap style and join style */
  sprintf (_plotter->page->point, "\
%d setlinecap %d setlinejoin\n",
	   _ps_cap_style[_plotter->drawstate->cap_type], 
	   _ps_join_style[_plotter->drawstate->join_type]);
  _update_buffer (_plotter->page);

  /* idraw instruction: brush type (spec'd as bit vector) */
  sprintf (_plotter->page->point, "\
%%I b %ld\n", 
	   _idraw_brush_pattern[_plotter->drawstate->line_type]);
  _update_buffer (_plotter->page);

  /* compute PS dash array for this line type (scale by line width
     if larger than 1 device unit, i.e., 1 point) */
  scale = _plotter->drawstate->device_line_width;
  if (scale < 1.0)
    scale = 1.0;
  for (i = 0; i < PS_DASH_ARRAY_LEN; i++)
    dashbuf[i] = scale * _ps_dash_array[_plotter->drawstate->line_type][i];

  /* PS instruction: SetB (i.e. setbrush), with args
     LineWidth, LeftArrow, RightArrow, DashArray, DashOffset. */
  /* Note LineWidth must be an integer for idraw compatibility. */
  if (_plotter->drawstate->line_type == L_SOLID)
    sprintf (_plotter->page->point, "\
%d 0 0 [ ] 0 SetB\n", 
  	     _plotter->drawstate->quantized_device_line_width);
  else
    /* emit dash array.  THIS ASSUMES PS_DASH_ARRAY_LEN = 4. */
    sprintf (_plotter->page->point, "\
%d 0 0 [ %.3g %.3g %.3g %.3g ] 0 SetB\n", 
  	     _plotter->drawstate->quantized_device_line_width,
             dashbuf[0], dashbuf[1], dashbuf[2], dashbuf[3]);
  _update_buffer (_plotter->page);
  
  /* idraw instruction: set foreground color */
  _plotter->set_pen_color();  /* invoked lazily, i.e. when needed */
  sprintf (_plotter->page->point, "\
%%I cfg %s\n\
%g %g %g SetCFg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red, 
	   _plotter->drawstate->ps_fgcolor_green, 
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer (_plotter->page);
  
  /* idraw instruction: set background color */
  _plotter->set_fill_color();  /* invoked lazily, i.e. when needed */
  sprintf (_plotter->page->point, "\
%%I cbg %s\n\
%g %g %g SetCBg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_bgcolor],
	   _plotter->drawstate->ps_fillcolor_red, 
	   _plotter->drawstate->ps_fillcolor_green, 
	   _plotter->drawstate->ps_fillcolor_blue);
  _update_buffer (_plotter->page);
  
  /* includes idraw instruction: set fill pattern */
  if (_plotter->drawstate->fill_level == 0)	/* transparent */
    sprintf (_plotter->page->point, "\
%%I p\n\
none SetP\n");
  else	/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_plotter->page->point, "\
%%I p\n\
%f SetP\n", 
	     _idraw_stdshadings[_plotter->drawstate->idraw_shading]);
  _update_buffer (_plotter->page);
  
  /* includes idraw instruction: transformation matrix (all 6 elements) */
  strcpy (_plotter->page->point, "\
%I t\n\
["); 
  _update_buffer (_plotter->page);
  for (i = 0; i < 6; i++)
    {
      if ((i==0) || (i==1) || (i==2) || (i==3))
	sprintf (_plotter->page->point, "%.7g ", _plotter->drawstate->transform.m[i] / granularity);
      else
	sprintf (_plotter->page->point, "%.7g ", _plotter->drawstate->transform.m[i]);
      _update_buffer (_plotter->page);
    }
  strcpy (_plotter->page->point, "\
] concat\n");
  _update_buffer (_plotter->page);
  
  /* idraw instruction: number of points in line */
  sprintf (_plotter->page->point, "\
%%I %d\n", 
	   _plotter->drawstate->points_in_path - (closed ? 1 : 0));
  _update_buffer (_plotter->page);

  linewidth = _plotter->drawstate->line_width;
  /* number of points to be output */
  numpoints = _plotter->drawstate->points_in_path - (closed ? 1 : 0);
  for (i=0; i < numpoints; i++)
    {
      /* output the data point */
      sprintf (_plotter->page->point, "\
%d %d\n",
	       IROUND(granularity * (_plotter->drawstate->datapoints[i]).x),
	       IROUND(granularity * (_plotter->drawstate->datapoints[i]).y));
      _update_buffer (_plotter->page);
      
      /* update bounding box */
      if (!closed && ((i == 0) || (i == numpoints - 1)))
	/* an end rather than a join */
	{
	  int otherp;
	  
	  otherp = (i == 0 ? 1 : numpoints - 2);
	  _set_line_end_bbox (_plotter->page,
			      _plotter->drawstate->datapoints[i].x,
			      _plotter->drawstate->datapoints[i].y,
			      _plotter->drawstate->datapoints[otherp].x,
			      _plotter->drawstate->datapoints[otherp].y,
			      linewidth, _plotter->drawstate->cap_type);
	}
      else
	/* a join rather than an end */
	{
	  int a, b, c;

	  if (closed && i == 0)	/* wrap */
	    {
	      a = numpoints - 1;
	      b = 0;
	      c = 1;
	    }
	  else			/* normal join */
	    {
	      a = i - 1;
	      b = i;
	      c = i + 1;
	    }
	  _set_line_join_bbox(_plotter->page,
			      _plotter->drawstate->datapoints[a].x,
			      _plotter->drawstate->datapoints[a].y,
			      _plotter->drawstate->datapoints[b].x,
			      _plotter->drawstate->datapoints[b].y,
			      _plotter->drawstate->datapoints[c].x,
			      _plotter->drawstate->datapoints[c].y,
			      linewidth, _plotter->drawstate->join_type);
	}
    }
  
  if (closed)
    sprintf (_plotter->page->point, "\
%d Poly\n\
End\n\n", numpoints);
  else
    sprintf (_plotter->page->point, "\
%d MLine\n\
End\n\n", numpoints);
  _update_buffer (_plotter->page);
  
  /* reset path storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->points_in_path = 0;

  return 0;
}
