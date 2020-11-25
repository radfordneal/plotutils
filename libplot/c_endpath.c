/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for CGMPlotters.  If the parameter CGM_MAX_VERSION is
   "1", at the time endpath() is invoked, the only possible contents of the
   the path storage buffer are a sequence of line segments.  If
   CGM_MAX_VERSION is "2", the path buffer may also contain a single
   elliptic or circular arc.  If CGM_MAX_VERSION is "3", the path buffer
   may contain an arbitrary sequence of these three primitives, and cubic
   Beziers too, i.e., an arbitrary `mixed path'.

   These restrictions on the path buffer contents are implemented by
   setting internal Plotter parameters at initialization time (e.g.,
   _plotter->have_mixed_paths; see c_defplot.c).  The reason for the
   restrictions is obvious: to store in the path buffer only those
   primitives that can be represented by a single CGM object, either simple
   or compound.  For example, circular arcs are not stored unless the
   emitting of version-2 CGM primitives is supported.  That's because
   although version-1 CGM's support counterclockwise arcs, clockwise arcs
   are supported only beginning with version 2.  And even though version-2
   CGM's support arbitrary closed mixed paths, at least ones that don't
   contain Beziers ("closed figures"), arbitrary open mixed paths
   ("compound lines") are supported only beginning with version 3. */

#include "sys-defines.h"
#include "extern.h"

typedef struct
{
  int x, y;
  int xc, yc;			/* center for arcs, control pt. for S_CUBIC */
  int xd, yd;			/* second control point (S_CUBIC only) */
  int type;			/* S_LINE or S_ARC or S_ELLARC or S_CUBIC */
} plGeneralizedIntPoint;

int
#ifdef _HAVE_PROTOS
_c_endpath (S___(Plotter *_plotter))
#else
_c_endpath (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool closed;
  plGeneralizedIntPoint *xarray;
  int polyline_len;
  bool draw_as_cgm_compound, path_is_single_polyline;
  int pass;
  int first_element_type;
  int i, byte_count, data_byte_count, data_len;
  int desired_interior_style;
  const char *desired_interior_style_string;
  
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
  
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    {
      /* reset path storage buffer and return */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;
      return 0;
    }

  /* set CGM line attributes, by emitting appropriate commands (pass hint) */
  _plotter->drawstate->cgm_object_type = (closed ? CGM_OBJECT_CLOSED : CGM_OBJECT_OPEN);
  _plotter->set_pen_color (S___(_plotter));
  _plotter->set_fill_color (S___(_plotter));
  _plotter->set_attributes (S___(_plotter));

  /* array for points, with positions expressed in integer device coors */
  xarray = (plGeneralizedIntPoint *)_plot_xmalloc (_plotter->drawstate->points_in_path * sizeof(plGeneralizedIntPoint));

  /* add first point of path to xarray[] (type field is meaningless) */
  xarray[0].x = IROUND(XD(_plotter->drawstate->datapoints[0].x, 
			  _plotter->drawstate->datapoints[0].y));
  xarray[0].y = IROUND(YD(_plotter->drawstate->datapoints[0].x, 
			  _plotter->drawstate->datapoints[0].y));
  polyline_len = 1;

  /* convert to integer CGM coordinates (unlike the HP-GL case [see
     h_endpath.c], we don't remove runs, so after this loop completes,
     polyline_len equals _plotter->drawstate->points_in_path) */
     
  for (i = 1; i < _plotter->drawstate->points_in_path; i++)
    {
      plGeneralizedPoint datapoint;
      double xuser, yuser, xdev, ydev;
      int device_x, device_y;
	  
      datapoint = _plotter->drawstate->datapoints[i];
      xuser = datapoint.x;
      yuser = datapoint.y;
      xdev = XD(xuser, yuser);
      ydev = YD(xuser, yuser);
      device_x = IROUND(xdev);
      device_y = IROUND(ydev);

      {
	int element_type;
	int device_xc, device_yc;
	
	xarray[polyline_len].x = device_x;
	xarray[polyline_len].y = device_y;
	element_type = datapoint.type;
	xarray[polyline_len].type = element_type;
	
	if (element_type == S_ARC || element_type == S_ELLARC)
	  /* an arc or elliptic arc element, so compute center too */
	  {
	    device_xc = IROUND(XD(datapoint.xc, datapoint.yc));
	    device_yc = IROUND(YD(datapoint.xc, datapoint.yc));
	    xarray[polyline_len].xc = device_xc;
	    xarray[polyline_len].yc = device_yc;
	  }
	else if (element_type == S_CUBIC)
	  /* a cubic Bezier element, so compute control points too */
	  {
	    xarray[polyline_len].xc 
	      = IROUND(XD(datapoint.xc, datapoint.yc));
	    xarray[polyline_len].yc 
	      = IROUND(YD(datapoint.xc, datapoint.yc));
	    xarray[polyline_len].xd
	      = IROUND(XD(datapoint.xd, datapoint.yd));
	    xarray[polyline_len].yd
	      = IROUND(YD(datapoint.xd, datapoint.yd));
	  }
	
	polyline_len++;
      }
    }

  /* A hack for CGM: if a circular or elliptic arc segment in integer
     device coordinates looks bogus, i.e. endpoints are the same or either
     is the same as the center point, replace it by a line segment.  This
     will allow us to assume, later, that the displacement vectors from the
     center to the two endpoints are nonzero and unequal. */

  for (i = 1; i < polyline_len; i++)
    {
      if (xarray[i].type == S_ARC || xarray[i].type == S_ELLARC)
	if ((xarray[i-1].x == xarray[i].x && xarray[i-1].y == xarray[i].y)
	    || (xarray[i-1].x == xarray[i].xc && xarray[i-1].y == xarray[i].yc)
	    || (xarray[i].x == xarray[i].xc && xarray[i].y == xarray[i].yc))
	  xarray[i].type = S_LINE;
    }

  /* set CGM attributes (differently, depending on whether path is closed
     or open, because different CGM graphical primitives will be emitted in
     the two cases to draw the path) */

  if (closed)
    {
      if (_plotter->drawstate->fill_type == 0)
	/* won't do filling */
	{
	  desired_interior_style = CGM_INT_STYLE_EMPTY;
	  desired_interior_style_string = "empty";
	}
      else
	/* will do filling */
	{
	  desired_interior_style = CGM_INT_STYLE_SOLID;
	  desired_interior_style_string = "solid";
	}

      if (_plotter->cgm_interior_style != desired_interior_style)
	/* emit "INTERIOR STYLE" command */
	{
	  data_len = 2;	/* 2 bytes per enum */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 22,
				    data_len, &byte_count,
				    "INTSTYLE");
	  _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			  desired_interior_style,
			  data_len, &data_byte_count, &byte_count,
			  desired_interior_style_string);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update interior style */
	  _plotter->cgm_interior_style = desired_interior_style;
	}
      
      if (_plotter->drawstate->pen_type)
	/* should draw the closed path so that edge is visible */
	{
	  if (_plotter->cgm_edge_is_visible != true)
	    /* emit "EDGE VISIBILITY" command */
	    {
	      data_len = 2;	/* 2 bytes per enum */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 30,
					data_len, &byte_count,
					"EDGEVIS");
	      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			      1,
			      data_len, &data_byte_count, &byte_count,
			      "on");
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update edge visibility */
	      _plotter->cgm_edge_is_visible = true;
	    }
	}
      else
	/* shouldn't edge the closed path */
	{
	  if (_plotter->cgm_edge_is_visible != false)
	    /* emit "EDGE VISIBILITY" command */
	    {
	      data_len = 2;	/* 2 bytes per enum */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 30,
					data_len, &byte_count,
					"EDGEVIS");
	      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			      0,
			      data_len, &data_byte_count, &byte_count,
			      "off");
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update edge visibility */
	      _plotter->cgm_edge_is_visible = false;
	    }
	}
    }
  else
    /* open! */
    {
      if (_plotter->drawstate->fill_type != 0)
	/* will `fill' the path by first drawing an edgeless solid-filled
	   polygon, or an edgeless solid-filled closed figure; in both
	   cases edge visibility will be turned off */
	{
	  if (_plotter->cgm_interior_style != CGM_INT_STYLE_SOLID)
	    /* emit "INTERIOR STYLE" command */
	    {
	      data_len = 2;	/* 2 bytes per enum */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 22,
					data_len, &byte_count,
					"INTSTYLE");
	      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			      CGM_INT_STYLE_SOLID,
			      data_len, &data_byte_count, &byte_count,
			      "solid");
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update interior style */
	      _plotter->cgm_interior_style = CGM_INT_STYLE_SOLID;
	    }
      
	  if (_plotter->cgm_edge_is_visible)
	    /* emit "EDGE VISIBILITY" command */
	    {
	      data_len = 2;	/* 2 bytes per enum */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 30,
					data_len, &byte_count,
					"EDGEVIS");
	      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			      0,
			      data_len, &data_byte_count, &byte_count,
			      "off");
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update edge visibility */
	      _plotter->cgm_edge_is_visible = false;
	    }
	}
    }

  /* Will path be drawn as a CGM compound primitive, containing > 1
     graphical primitives?  If it contains more than one type of path
     segment, or if it contains more than a single circular arc segment or
     elliptic arc segment, answer is `yes'.

     Because of our policies, implemented elsewhere, on what may be stored
     in the path buffer (see above), we'll draw as a compound primitive
     only if CGM_MAX_VERSION >= 3. */

  draw_as_cgm_compound = false;
  first_element_type = xarray[1].type;
  for (i = 2; i < polyline_len; i++)
    {
      if (xarray[i].type == S_ARC || xarray[i].type == S_ELLARC
	  || xarray[i].type != first_element_type)
	{
	  draw_as_cgm_compound = true;
	  break;
	}
    }

  /* is path simply a polyline? */
    {
      path_is_single_polyline = true;
      for (i = 1; i < polyline_len; i++)
	{
	  if (xarray[i].type != S_LINE)
	    {
	      path_is_single_polyline = false;
	      break;
	    }
	}
    }

  /* Make two passes through path buffer: (0) draw and fill, if necessary,
     a closed CGM object, e.g. a `closed figure' [necessary iff path is
     closed, or is open and filled], and (1) edge an open CGM object,
     e.g. a `compound line' [necessary iff path is open]. */

  for (pass = 0; pass < 2; pass++)
    {
      int primitives_emitted;

      if (pass == 0 && !(closed || _plotter->drawstate->fill_type != 0))
	/* no drawing of a closed object needed: skip pass 0 */
	continue;
      
      if (pass == 1 && (closed || (!closed && _plotter->drawstate->pen_type == 0)))
	/* no need for a special `draw edge' pass: skip pass 1 */
	continue;

      /* keep track of individual graphical primitives emitted per pass
         (profile requires <=128 per composite primitive, closed or open) */
      primitives_emitted = 0;

      if (pass == 0 && !path_is_single_polyline)
	/* emit `BEGIN CLOSED FIGURE' command (no parameters); drawing of
	   closed polylines and filling of open ones is handled specially
	   (see below) */
	{
	  data_len = 0;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_DELIMITER_ELEMENT, 8,
				    data_len, &byte_count,
				    "BEGFIGURE");
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update CGM version needed for this page */
	  _plotter->cgm_page_version = IMAX(2, _plotter->cgm_page_version);
	}
      
      if (pass == 1 && draw_as_cgm_compound)
	/* emit `BEGIN COMPOUND LINE' command (no parameters) */
	{
	  data_len = 0;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_DELIMITER_ELEMENT, 15,
				    data_len, &byte_count,
				    "BEGCOMPOLINE");
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update CGM version needed for this page */
	  _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	}
      
      /* iterate over path elements, combining runs of line segments into
	 polylines, and runs of Beziers into poly-Beziers, but emitting
	 each circular arc and elliptic arc individually (since CGM doesn't
	 support poly-arcs) */
      i = 0;
      while (i + 1 < polyline_len)
	{
	  int j, element_type, end_of_run;
	  
	  /* determine `run' (relevant only for lines, Beziers) */
	  element_type = xarray[i + 1].type;
	  for (j = i + 1; 
	       j < polyline_len && xarray[j].type == element_type; 
	       j++)
	    ;
	  end_of_run = j - 1;
	  
	  switch (element_type)
	    {
	    case S_LINE:
	      if ((pass == 0 && !path_is_single_polyline) || (pass == 1))
		/* normal case: emit "POLYLINE" command to draw a polyline */
		/* number of line segments in polyline: end_of_run - i */
		/* number of points in polyline: 1 + (end_of_run - i) */
		{
		  /* update CGM profile for this page */
		  if (1 + (end_of_run - i) > 4096)
		    _plotter->cgm_page_profile = 
		      IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
		  
		  data_len = 2 * CGM_BINARY_BYTES_PER_INTEGER * (1 + end_of_run - i);
		  byte_count = data_byte_count = 0;
		  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 1,
					    data_len, &byte_count,
					    "LINE");
		  /* combine line segments into polyline */
		  for ( ; i <= end_of_run; i++)
		    _cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				     xarray[i].x, xarray[i].y,
				     data_len, &data_byte_count, &byte_count);
		  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
						&byte_count);
		  primitives_emitted++;
		  /* next CGM component object begins at i=end_of_run */
		  i--;
		}
	      else
		/* Special case: we're running pass 0, and path consists of
		   a single polyline.  So emit "POLYGON" command, omitting
		   the final point if the polyline is closed, to agree with
		   CGM conventions.  */
		{
		  /* update CGM profile for this page */
		  if (polyline_len - (closed ? 1 : 0) > 4096)
		    _plotter->cgm_page_profile = 
		      IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
		  
		  data_len = 2 * CGM_BINARY_BYTES_PER_INTEGER * (polyline_len - (closed ? 1 : 0));
		  byte_count = data_byte_count = 0;
		  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 7,
					    data_len, &byte_count,
					    "POLYGON");
		  for (i = 0; i < polyline_len - (closed ? 1 : 0); i++)
		    _cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				     xarray[i].x, xarray[i].y,
				     data_len, &data_byte_count, &byte_count);
		  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
						&byte_count);
		  primitives_emitted++;
		  
		  /* we've used up the entire path buffer: no more
		     primitives to emit */
		  i = polyline_len - 1;
		}
	      break;
	      
	    case S_ARC:
	      /* emit "CIRCULAR ARC CENTRE [REVERSED]" command */
	      {
		int delta0_x = xarray[i].x - xarray[i + 1].xc;
		int delta0_y = xarray[i].y - xarray[i + 1].yc;
		int delta1_x = xarray[i + 1].x - xarray[i + 1].xc;
		int delta1_y = xarray[i + 1].y - xarray[i + 1].yc;
		double radius = sqrt((double)delta0_x * (double)delta0_x
				     + (double)delta0_y * (double)delta0_y);
		int i_radius = IROUND(radius);
		double dot = ((double)delta0_x * (double)delta1_y 
			      - (double)delta0_y * (double)delta1_x);
		bool reversed = (dot >= 0.0 ? false : true);
		
		/* args: 1 point, 2 vectors, and the radius */
		data_len = (3 * 2 + 1) * CGM_BINARY_BYTES_PER_INTEGER;
		byte_count = data_byte_count = 0;
		if (reversed)
		  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 20,
					    data_len, &byte_count,
					    "ARCCTRREV");
		else
		  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 15,
					    data_len, &byte_count,
					    "ARCCTR");
		/* center point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i + 1].xc, xarray[i + 1].yc,
				 data_len, &data_byte_count, &byte_count);
		/* vector from center to starting point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 delta0_x, delta0_y,
				 data_len, &data_byte_count, &byte_count);
		/* vector from center to ending point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 delta1_x, delta1_y,
				 data_len, &data_byte_count, &byte_count);
		/* radius (distance from center to starting point) */
		_cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
				   i_radius,
				   data_len, &data_byte_count, &byte_count);
		_cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					      &byte_count);
		primitives_emitted++;

		/* update CGM version needed for this page */
		if (reversed)
		  _plotter->cgm_page_version = IMAX(2, _plotter->cgm_page_version);
	      }
	      /* on to next CGM component object */
	      i++;
	      break;
	      
	    case S_ELLARC:
	      /* emit "ELLIPTICAL ARC" command to draw quarter-ellipse */
	      {
		/* args: 3 points, 2 vectors */
		data_len = 5 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 18,
					  data_len, &byte_count,
					  "ELLIPARC");
		/* center point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i + 1].xc, xarray[i + 1].yc,
				 data_len, &data_byte_count, &byte_count);
		/* starting point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i].x, xarray[i].y,
				 data_len, &data_byte_count, &byte_count);
		/* ending point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i + 1].x, xarray[i + 1].y,
				 data_len, &data_byte_count, &byte_count);
		/* vector from center to starting point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i].x - xarray[i + 1].xc,
				 xarray[i].y - xarray[i + 1].yc,
				 data_len, &data_byte_count, &byte_count);
		/* vector from center to ending point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i + 1].x - xarray[i + 1].xc, 
				 xarray[i + 1].y - xarray[i + 1].yc,
				 data_len, &data_byte_count, &byte_count);
		_cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					      &byte_count);
		primitives_emitted++;
	      }
	      /* on to next CGM component object */
	      i++;
	      break;
	      
	    case S_CUBIC:
	      /* emit "POLYBEZIER" command */
	      /* number of Bezier segments in path: end_of_run - i */
	      /* number of points in path:  1 + 3 * (end_of_run - i) */
	      /* Note: arguments include also a single `continuity
		 indicator' (a two-byte CGM index) */
	      {
		/* update CGM profile for this page */
		if (1 + 3 * (end_of_run - i) > 4096)
		  _plotter->cgm_page_profile = 
		    IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);

		data_len = 2 + (2 * CGM_BINARY_BYTES_PER_INTEGER) * (1 + 3 * (end_of_run - i));
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 26,
					  data_len, &byte_count,
					  "POLYBEZIER");
		_cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
				 /* poly-Bezier continuity index: `2' means
				    successive Beziers abut, so (after the
				    first) each is specified by only three
				    points; `1' means they don't abut.  To
				    avoid stressing CGM interpreters, we
				    specify `1' if there's only 1 Bezier.  */
				 (end_of_run - i > 1 ? 2 : 1),
				 data_len, &data_byte_count, &byte_count);
		/* starting point */
		_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
				 xarray[i].x, xarray[i].y,
				 data_len, &data_byte_count, &byte_count);
		i++;
		/* combine Bezier segments into poly-Bezier */
		for ( ; i <= end_of_run; i++)
		      {
			_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
					 xarray[i].xc, xarray[i].yc,
					 data_len, &data_byte_count, &byte_count);
			_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
					 xarray[i].xd, xarray[i].yd,
					 data_len, &data_byte_count, &byte_count);
			_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
					 xarray[i].x, xarray[i].y,
					 data_len, &data_byte_count, &byte_count);
		      }
		_cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					      &byte_count);
		primitives_emitted++;
		
		/* update CGM version needed for this page */
		_plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
		
		/* next CGM component object begins at i=end_of_run */
		i--;
	      }
	      break;
	      
	    default:
	      /* shouldn't happen: unknown path segment type, ignore */
	      i++;
	      break;
	    }
	}

      if (pass == 0 && !path_is_single_polyline)
	/* emit `END CLOSED FIGURE' command (no parameters) */
	{
	  data_len = 0;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_DELIMITER_ELEMENT, 9,
				    data_len, &byte_count,
				    "ENDFIGURE");
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);

	  /* update CGM version needed for this page */
	  _plotter->cgm_page_version = IMAX(2, _plotter->cgm_page_version);
		    
	  /* update CGM profile for this page */
	  if (primitives_emitted > 128)
	    _plotter->cgm_page_profile = 
	      IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
	}
      
      if (pass == 1 && draw_as_cgm_compound)
	/* emit `END COMPOUND LINE' command (no parameters) */
	{
	  data_len = 0;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_DELIMITER_ELEMENT, 16,
				    data_len, &byte_count,
				    "ENDCOMPOLINE");
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);

	  /* update CGM version needed for this page */
	  _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);

	  /* update CGM profile for this page */
	  if (primitives_emitted > 128)
	    _plotter->cgm_page_profile = 
	      IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
	}

    } /* end of loop over passes */
  
  /* free arrays of device-frame points */
  free (xarray);

  /* reset path storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->points_in_path = 0;

  return 0;
}
