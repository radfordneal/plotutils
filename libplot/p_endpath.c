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
#include "extern.h"

/* Each dash and gap in our canonical line modes ("shortdashed",
   "dotdashed") etc. has length that we take to be an integer multiple of
   the line width.  (For the integers, see g_dash2.c).  Actually, when
   performing this computation we impose a floor on the line width (so that
   measured in the device frame, the following is the minimum value for the
   nominal line width). */
#define MIN_NOMINAL_LINE_WIDTH_IN_POINTS 1.0

/* 16-bit brush patterns for idraw (1 = on, 0 = off), indexed by our
   internal numbering of line styles, i.e. by L_{SOLID,DOTTED,DOTDASHED,
   SHORTDASHED,LONGDASHED,DOTDOTDASHED,DOTDOTDOTDASHED} */ 
const long _idraw_brush_pattern[NUM_LINE_STYLES] = 
{ 0xffff, 0x8888, 0xfc30, 0xf0f0, 0xffc0, 0xfccc, 0xfdb6 };

/* PS join styles, indexed by internal number (miter/rd./bevel/triangular) */
const int _ps_join_style[] =
{ PS_LINE_JOIN_MITER, PS_LINE_JOIN_ROUND, PS_LINE_JOIN_BEVEL, PS_LINE_JOIN_ROUND };

/* PS cap styles, indexed by internal number (butt/rd./project/triangular) */
const int _ps_cap_style[] =
{ PS_LINE_CAP_BUTT, PS_LINE_CAP_ROUND, PS_LINE_CAP_PROJECT, PS_LINE_CAP_ROUND };

int
#ifdef _HAVE_PROTOS
_p_endpath (S___(Plotter *_plotter))
#else
_p_endpath (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool closed;
  int i;
  
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "endpath: invalid operation");
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
  
  /* general case: points are vertices of a polyline */
  
  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* have something to draw */
    {
      int numpoints, index_start, index_increment;
      double granularity, line_width;

      /* includes idraw instruction: start of MLine or Poly */
      if (closed)
	strcpy (_plotter->page->point, "Begin %I Poly\n");
      else
	strcpy (_plotter->page->point, "Begin %I MLine\n");
      _update_buffer (_plotter->page);

      granularity = _p_emit_common_attributes (S___(_plotter));

      /* includes idraw instruction: transformation matrix (all 6 elements) */
      strcpy (_plotter->page->point, "%I t\n["); 
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

      line_width = _plotter->drawstate->line_width;
      /* number of points to be output */
      numpoints = _plotter->drawstate->points_in_path - (closed ? 1 : 0);

      /* if polyline is closed, loop through points _backward_, since the
	 `Poly' function in the idraw prologue draws closed polylines in
	 reverse, and we want the dasharray to be interpreted correctly */
      if (closed)
	{
	  index_start = numpoints - 1;
	  index_increment = -1;
	}
      else
	{
	  index_start = 0;
	  index_increment = 1;
	}
      for (i = index_start; i >= 0 && i <= numpoints - 1; i += index_increment)
	{
	  /* output the data point */
	  sprintf (_plotter->page->point, "\
%d %d\n",
		   IROUND(granularity * _plotter->drawstate->datapoints[i].x),
		   IROUND(granularity * _plotter->drawstate->datapoints[i].y));
	  _update_buffer (_plotter->page);
      
	  /* update bounding box */
	  if (!closed && ((i == 0) || (i == numpoints - 1)))
	    /* an end rather than a join */
	    {
	      int otherp;
	  
	      otherp = (i == 0 ? 1 : numpoints - 2);
	      _set_line_end_bbox (R___(_plotter)
				  _plotter->page,
				  _plotter->drawstate->datapoints[i].x,
				  _plotter->drawstate->datapoints[i].y,
				  _plotter->drawstate->datapoints[otherp].x,
				  _plotter->drawstate->datapoints[otherp].y,
				  line_width, _plotter->drawstate->cap_type);
	    }
	  else
	    /* a join rather than an end */
	    {
	      int a, b, c;

	      if (closed && i == 0) /* wrap */
		{
		  a = numpoints - 1;
		  b = 0;
		  c = 1;
		}
	      else		/* normal join */
		{
		  a = i - 1;
		  b = i;
		  c = i + 1;
		}
	      _set_line_join_bbox(R___(_plotter)
				  _plotter->page,
				  _plotter->drawstate->datapoints[a].x,
				  _plotter->drawstate->datapoints[a].y,
				  _plotter->drawstate->datapoints[b].x,
				  _plotter->drawstate->datapoints[b].y,
				  _plotter->drawstate->datapoints[c].x,
				  _plotter->drawstate->datapoints[c].y,
				  line_width,
				  _plotter->drawstate->join_type,
				  _plotter->drawstate->miter_limit);
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
    }
  
  /* reset path storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->points_in_path = 0;

  return 0;
}

/* Emit the common attributes, for PS and idraw, of any path or ellipse
   object.  This includes the CTM, fill rule, cap and join styles and miter
   limit, dash array, foreground and background colors, and idraw brush.
   
   Return value is the `granularity': a factor by which user-frame
   coordinates, when emitted to the output file as integers, should be
   scaled up.  This is to avoid loss of precision when using integer
   coordinates.  The CTM emitted here will automatically compensate for
   this factor. */

double
#ifdef _HAVE_PROTOS
_p_emit_common_attributes (S___(Plotter *_plotter))
#else
_p_emit_common_attributes (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool singular_map;
  int i;
  double invnorm = 0.0, granularity = 1.0;
  double linewidth_adjust = 1.0;
  double min_sing_val, max_sing_val, norm;

  /* compute norm of user->device affine transformation */

  /* This minimum singular value isn't really the norm.  But it's the
     nominal device-frame line width divided by the actual user-frame
     line-width (see g_linewidth.c), and that's what we need. */
  _matrix_sing_vals (_plotter->drawstate->transform.m,
		     &min_sing_val, &max_sing_val);
  norm = min_sing_val;

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

  /* redefine `originalCTM' matrix, which is the CTM applied when the
     polyline is stroked (as opposed to drawn).  We define it to be the
     same as the one in effect when the polyline was drawn. */
  if (singular_map != true)
    {
      int integer_linewidth = _plotter->drawstate->quantized_device_line_width;
      double double_linewidth = _plotter->drawstate->device_line_width;

      /* adjustment to CTM needed, due to our specifying line widths as
         integers */
      if (integer_linewidth != 0)
	linewidth_adjust = double_linewidth / integer_linewidth;
      else
	linewidth_adjust = 1.0;

      strcpy (_plotter->page->point, "[");
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
  
  /* specify cap style and join style, and miter limit if mitering */
  if (_plotter->drawstate->join_type == JOIN_MITER)
    sprintf (_plotter->page->point, "\
%d setlinecap %d setlinejoin %.4g setmiterlimit\n",
	     _ps_cap_style[_plotter->drawstate->cap_type], 
	     _ps_join_style[_plotter->drawstate->join_type],
	     _plotter->drawstate->miter_limit);
  else
    sprintf (_plotter->page->point, "\
%d setlinecap %d setlinejoin\n",
	     _ps_cap_style[_plotter->drawstate->cap_type], 
	     _ps_join_style[_plotter->drawstate->join_type]);
  _update_buffer (_plotter->page);
  
  /* specify fill rule (i.e. whether to use even-odd filling) */
  if (_plotter->drawstate->fill_rule_type == FILL_NONZERO_WINDING)
    sprintf (_plotter->page->point, "\
/eoFillRule false def\n");
  else
    sprintf (_plotter->page->point, "\
/eoFillRule true def\n");
  _update_buffer (_plotter->page);
  
  if (_plotter->drawstate->pen_type != 0)
    /* pen is present, so will brush an outline of the path */
    {
      int num_dashes;
      double scale;
      double *dashbuf, dash_cycle_length, offset;

      if (_plotter->drawstate->dash_array_in_effect)
	/* have user-specified dash array */
	{
	  /* idraw instruction: brush type (spec'd as bit vector, but for now
	     we just use a solid brush */
	  sprintf (_plotter->page->point, "\
%%I b %ld\n", 
		   (long int)0xffff);
	  _update_buffer (_plotter->page);
	  
	  num_dashes = _plotter->drawstate->dash_array_len;
	  if (num_dashes > 0)
	    dashbuf = (double *)_plot_xmalloc (num_dashes * sizeof(double));
	  else
	    dashbuf = NULL;
	  /* take the adjustment to the CTM into account */
	  scale = norm / linewidth_adjust;
	  
	  dash_cycle_length = 0.0;
	  for (i = 0; i < num_dashes; i++)
	    {
	      double dashlen;
	      
	      dashlen = _plotter->drawstate->dash_array[i];
	      dash_cycle_length += dashlen;
	      dashbuf[i] = scale * dashlen;
	    }
	  
	  if (dash_cycle_length > 0.0)
	    /* choose an offset in range 0..true_cycle_length */
	    {
	      double true_cycle_length;
	      
	      offset = _plotter->drawstate->dash_offset;
	      true_cycle_length = 
		dash_cycle_length * (num_dashes % 2 == 1 ? 2 : 1);
	      while (offset < 0.0)
		offset += true_cycle_length;
	      offset = fmod (offset, true_cycle_length);
	      offset *= scale;
	    }
	  else
	    offset = 0.0;
	}
      else
	/* have one of the canonical line types */
	{
	  /* idraw brush type (spec'd as bit vector) */
	  sprintf (_plotter->page->point, "\
%%I b %ld\n", 
		   _idraw_brush_pattern[_plotter->drawstate->line_type]);
	  _update_buffer (_plotter->page);
	  
	  if (_plotter->drawstate->line_type == L_SOLID)
	    {
	      num_dashes = 0;
	      dashbuf = NULL;
	      offset = 0.0;
	    }
	  else
	    {
	      const int *dash_array;
	      double display_size_in_points, min_dash_unit;
	      
	      /* compute PS dash array for this line type */
	      dash_array = 
		_line_styles[_plotter->drawstate->line_type].dash_array;
	      num_dashes =
		_line_styles[_plotter->drawstate->line_type].dash_array_len;
	      dashbuf = (double *)_plot_xmalloc (num_dashes * sizeof(double));
	      
	      /* scale the array of integers by line width (actually by
		 floored line width; see comments at head of file) */
	      display_size_in_points = DMIN(_plotter->xmax - _plotter->xmin,
					    _plotter->ymax - _plotter->ymin);
	      min_dash_unit = (MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE 
			       * display_size_in_points);
	      scale = DMAX(min_dash_unit,
			   _plotter->drawstate->device_line_width);
	      /* take the adjustment to the CTM into account */
	      scale /= linewidth_adjust;
	      
	      for (i = 0; i < num_dashes; i++)
		dashbuf[i] = scale * dash_array[i];
	      offset = 0.0;
	    }
	}

      /* PS instruction: SetB (i.e. setbrush), with args
	 LineWidth, LeftArrow, RightArrow, DashArray, DashOffset. */
      /* Note LineWidth must be an integer for idraw compatibility. */
      
      /* emit dash array */
      sprintf (_plotter->page->point, "%d 0 0 [ ", 
	       _plotter->drawstate->quantized_device_line_width);
      _update_buffer (_plotter->page);
      for (i = 0; i < num_dashes; i++)
	{
	  sprintf (_plotter->page->point, "%.3g ", dashbuf[i]);
	  _update_buffer (_plotter->page);
	}
      sprintf (_plotter->page->point, "] %.3g SetB\n", offset);
      _update_buffer (_plotter->page);
      free (dashbuf);
    }
  else
    /* pen_type = 0, we have no pen to draw with (though we may do filling) */
    {
      sprintf (_plotter->page->point, "\
%%I b n\n\
none SetB\n");
      _update_buffer (_plotter->page);
    }
  
  /* idraw instruction: set foreground color */
  _plotter->set_pen_color (S___(_plotter)); /* invoked lazily, when needed */
  sprintf (_plotter->page->point, "\
%%I cfg %s\n\
%g %g %g SetCFg\n",
	   _idraw_stdcolornames[_plotter->drawstate->ps_idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red, 
	   _plotter->drawstate->ps_fgcolor_green, 
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer (_plotter->page);
  
  /* idraw instruction: set background color */
  _plotter->set_fill_color (S___(_plotter)); /* invoked lazily, when needed */
  sprintf (_plotter->page->point, "\
%%I cbg %s\n\
%g %g %g SetCBg\n",
	   _idraw_stdcolornames[_plotter->drawstate->ps_idraw_bgcolor],
	   _plotter->drawstate->ps_fillcolor_red, 
	   _plotter->drawstate->ps_fillcolor_green, 
	   _plotter->drawstate->ps_fillcolor_blue);
  _update_buffer (_plotter->page);
  
  /* includes idraw instruction: set fill pattern */
  if (_plotter->drawstate->fill_type == 0)	/* transparent */
    sprintf (_plotter->page->point, "\
%%I p\n\
none SetP\n");
  else			/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_plotter->page->point, "\
%%I p\n\
%f SetP\n", 
	     _idraw_stdshadings[_plotter->drawstate->ps_idraw_shading]);
  _update_buffer (_plotter->page);
  
  /* return factor we'll later use to scale up user-frame coordinates */
  return granularity;
}
