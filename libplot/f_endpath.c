/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for FigPlotters.  By construction, for FigPlotters our
   path storage buffer always includes either (1) a sequence of line
   segments, or (2) a single circular arc segment.  Those are the only two
   sorts of path that xfig can handle.  (For the latter to be included,
   the map from user to device coordinates must be uniform.) */

#include "sys-defines.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

/* Fig's line styles, indexed into by internal line number
   (L_SOLID/L_DOTTED/L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED/L_DOTDOTDASHED) */
const int _fig_line_style[NUM_LINE_STYLES] =
{ FIG_L_SOLID, FIG_L_DOTTED, FIG_L_DASHDOTTED, FIG_L_DASHED, FIG_L_DASHED,
    FIG_L_DASHDOUBLEDOTTED, FIG_L_DASHTRIPLEDOTTED };

/* Fig join styles, indexed by internal number (miter/rd./bevel/triangular) */
const int _fig_join_style[] =
{ FIG_JOIN_MITER, FIG_JOIN_ROUND, FIG_JOIN_BEVEL, FIG_JOIN_ROUND };

/* Fig cap styles, indexed by internal number (butt/rd./project/triangular) */
const int _fig_cap_style[] =
{ FIG_CAP_BUTT, FIG_CAP_ROUND, FIG_CAP_PROJECT, FIG_CAP_ROUND };

#define FUZZ 0.0000001

int
#ifdef _HAVE_PROTOS
_f_endpath (void)
#else
_f_endpath ()
#endif
{
  bool closed; 
  const char *format;
  int i, polyline_subtype, line_style;
  double nominal_spacing;

  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path == 2
      && _plotter->drawstate->datapoints[1].type == S_ARC)
    /* path buffer contains a single arc, not a polyline */
    {
      double x0 = _plotter->drawstate->datapoints[0].x;
      double y0 = _plotter->drawstate->datapoints[0].y;      
      double x1 = _plotter->drawstate->datapoints[1].x;
      double y1 = _plotter->drawstate->datapoints[1].y;      
      double xc = _plotter->drawstate->datapoints[1].xc;
      double yc = _plotter->drawstate->datapoints[1].yc;      

      _f_emit_arc (xc, yc, x0, y0, x1, y1);

      /* reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;
      return 0;
    }

  /* path buffer contains a polyline, not an arc */

  if (_plotter->drawstate->points_in_path == 0)	/* nothing to do */
    return 0;
  if (_plotter->drawstate->points_in_path == 1)	/* shouldn't happen */
    {
      /* reset path storage buffer */
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

  if (closed)
    {
      polyline_subtype = P_CLOSED;
      format = "#POLYLINE [CLOSED]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d";
    }
  else
    {
      polyline_subtype = P_OPEN;
      format = "#POLYLINE [OPEN]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d";
    }

  /* evaluate fig colors lazily, i.e. only when needed */
  _plotter->set_pen_color();
  _plotter->set_fill_color();
  
  /* compute line style (type of dotting/dashing, spacing of dots/dashes) */
  _f_compute_line_style (&line_style, &nominal_spacing);

  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  sprintf(_plotter->page->point,
	  format,
	  2,			/* polyline object */
	  polyline_subtype,	/* polyline subtype */
	  line_style,		/* Fig line style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  nominal_spacing,	/* style val, in Fig display units (float) */
	  _fig_join_style[_plotter->drawstate->join_type], /* join style */
	  _fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
	  0,			/* radius (of arc boxes, ignored here) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  _plotter->drawstate->points_in_path	/* number of points in line */
	  );
  _update_buffer (_plotter->page);

  for (i=0; i<_plotter->drawstate->points_in_path; i++)
    {
      GeneralizedPoint datapoint;
      double xu, yu, xd, yd;
      int device_x, device_y;

      datapoint = _plotter->drawstate->datapoints[i];
      xu = datapoint.x;
      yu = datapoint.y;
      xd = XD(xu, yu);
      yd = YD(xu, yu);
      device_x = IROUND(xd);
      device_y = IROUND(yd);

      if ((i%5) == 0)
	sprintf (_plotter->page->point, "\n\t"); /* make human-readable */
      else
	sprintf (_plotter->page->point, " ");
      _update_buffer (_plotter->page);

      sprintf (_plotter->page->point, "%d %d", device_x, device_y);
      _update_buffer (_plotter->page);
    }
  sprintf (_plotter->page->point, "\n");
  _update_buffer (_plotter->page);

  /* reset path storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->points_in_path = 0;

  return 0;
}

/* Emit Fig code for an arc.  This is called if the path buffer contains
   not a polyline, but a single circular arc.  If an arc was placed there,
   we can count on the map from the user frame to the device frame being
   isotropic (so the arc will be circular in the device frame too), and we
   can count on the arc not being of zero length. */

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

void
#ifdef _HAVE_PROTOS
_f_emit_arc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_f_emit_arc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point p0, p1, pc, pb;
  Vector v, v0, v1;
  double cross, radius, nominal_spacing;
  int line_style, orientation;

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

  /* cross product, zero means points are collinear */
  cross = v0.x * v1.y - v1.x * v0.y;

  /* Compute orientation.  Note libplot convention: if p0, p1, pc are
     collinear then arc goes counterclockwise from p0 to p1. */
  orientation = (cross >= 0.0 ? 1 : -1);

  radius = DIST(pc, p0);	/* radius is distance to p0 or p1 */

  v.x = p1.x - p0.x;		/* chord vector from p0 to p1 */
  v.y = p1.y - p0.y;
      
  _vscale(&v, radius);
  pb.x = pc.x + orientation * v.y; /* bisection point of arc */
  pb.y = pc.y - orientation * v.x;
      
  /* evaluate fig colors lazily, i.e. only when needed */
  _plotter->set_pen_color();
  _plotter->set_fill_color();
  
  /* compute line style (type of dotting/dashing, spacing of dots/dashes) */
  _f_compute_line_style (&line_style, &nominal_spacing);

  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  /* compute orientation in device frame */
  orientation *= (_plotter->drawstate->transform.nonreflection ? 1 : -1);

  if (orientation == -1)
    /* interchange p0, p1 (since xfig insists that p0, pb, p1 must appear
       in counterclockwise order around the arc) */
    {
      Point ptmp;
      
      ptmp = p0;
      p0 = p1;
      p1 = ptmp;
    }

  sprintf(_plotter->page->point,
	  "#ARC\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %.3f %.3f %d %d %d %d %d %d\n",
	  5,			/* arc object */
	  1,			/* open-ended arc subtype */
	  line_style,		/* Fig line style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  nominal_spacing,	/* style val, in Fig display units (float) */
	  _fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
	  1,			/* counterclockwise */
	  0,			/* no forward arrow */
	  0,			/* no backward arrow */
	  XD(pc.x, pc.y),	/* center_x (float) */
	  YD(pc.x, pc.y),	/* center_y (float) */
	  IROUND(XD(p0.x, p0.y)), /* 1st point user entered (p0) */
	  IROUND(YD(p0.x, p0.y)), 
	  IROUND(XD(pb.x, pb.y)), /* 2nd point user entered (bisection point)*/
	  IROUND(YD(pb.x, pb.y)),
	  IROUND(XD(p1.x, p1.y)), /* last point user entered (p1) */
	  IROUND(YD(p1.x, p1.y)));
  _update_buffer (_plotter->page);

  return;
}

/* compute appropriate Fig line style, and also appropriate value for Fig's
   notion of `dash length/dot gap' (in Fig display units) */
void
#ifdef _HAVE_PROTOS
_f_compute_line_style (int *style, double *spacing)
#else
_f_compute_line_style (style, spacing)
     int *style;
     double *spacing;
#endif
{
  int fig_line_style;
  double fig_nominal_spacing;
    
  if (_plotter->drawstate->dash_array_in_effect
      && _plotter->drawstate->dash_array_len == 2
      && (_plotter->drawstate->dash_array[1]
	  == _plotter->drawstate->dash_array[0]))
    /* special case of user-specified dashing (equal on/off lengths);
       we map this into Fig's `dashed' line type */
    {
      double min_sing_val, max_sing_val;

      /* Minimum singular value is the nominal device-frame line width
	 divided by the actual user-frame line-width (see g_linewidth.c),
	 so it's the user->device frame conversion factor. */
      _matrix_sing_vals (_plotter->drawstate->transform.m,
			 &min_sing_val, &max_sing_val);

      /* desired cycle length in Fig display units */
      fig_nominal_spacing =
	FIG_UNITS_TO_FIG_DISPLAY_UNITS(min_sing_val * 2.0 * _plotter->drawstate->dash_array[0]);
      fig_line_style = FIG_L_DASHED;
    }
  else if (_plotter->drawstate->dash_array_in_effect
	   && _plotter->drawstate->dash_array_len == 2
	   && (_plotter->drawstate->dash_array[1]
	       > (3 - FUZZ) * _plotter->drawstate->dash_array[0])
	   && (_plotter->drawstate->dash_array[1]
	       < (3 + FUZZ) * _plotter->drawstate->dash_array[0]))
    /* special case of user-specified dashing (gap length = 3 * dash length);
       we map this into Fig's `dotted' line type, since it agrees with
       libplot's convention for dashing `dotted' lines (see g_dash2.c) */
    {
      double min_sing_val, max_sing_val;

      _matrix_sing_vals (_plotter->drawstate->transform.m,
			 &min_sing_val, &max_sing_val);

      /* desired cycle length in Fig display units */
      fig_nominal_spacing =
	FIG_UNITS_TO_FIG_DISPLAY_UNITS(min_sing_val * 4.0 * _plotter->drawstate->dash_array[0]);
      fig_line_style = FIG_L_DOTTED;
    }
  else
    /* canonical line type; retrieve dash array from database (in g_dash2.c) */
    {
      int i, num_dashes, cycle_length;
      const int *dash_array;
      double display_size_in_fig_units, min_dash_unit, dash_unit;
      Displaycoors info;

      num_dashes =
	_line_styles[_plotter->drawstate->line_type].dash_array_len;
      dash_array = _line_styles[_plotter->drawstate->line_type].dash_array;
      cycle_length = 0;
      for (i = 0; i < num_dashes; i++)
	cycle_length += dash_array[i];
      /* multiply cycle length of dash array by device-frame line width in
	 Fig display units, with a floor on the latter (see comments at
	 head of file) */
      info = _plotter->display_coors;
      display_size_in_fig_units = (_plotter->device_units_per_inch
				   * DMIN(info.right - info.left, 
				      info.bottom - info.top)); /* flipped y */
      min_dash_unit = MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE 
	* FIG_UNITS_TO_FIG_DISPLAY_UNITS(display_size_in_fig_units);
      dash_unit = DMAX(min_dash_unit, _plotter->drawstate->device_line_width);

      /* desired cycle length in Fig display units */
      fig_nominal_spacing = cycle_length * dash_unit;
      fig_line_style = _fig_line_style[_plotter->drawstate->line_type];
    }
      
  /* compensate for Fig's (or fig2dev's) peculiarities; value stored in Fig
     output file isn't really the cycle length */
  switch (fig_line_style)
    {
    case FIG_L_SOLID:
    default:			/* shouldn't happen */
      break;
    case FIG_L_DOTTED:
      fig_nominal_spacing -= 1.0;
      break;
    case FIG_L_DASHDOTTED:
      fig_nominal_spacing -= 1.0;
      /* fall thru */
    case FIG_L_DASHED:
      fig_nominal_spacing *= 0.5;
      break;
    case FIG_L_DASHDOUBLEDOTTED:
      fig_nominal_spacing -= 2.0;
      fig_nominal_spacing /= (1.9 + 1/3.0); /* really */
      break;
    case FIG_L_DASHTRIPLEDOTTED:
      fig_nominal_spacing -= 3.0;
      fig_nominal_spacing /= 2.4;
      break;
    }
  if (fig_nominal_spacing <= 1.0)
    fig_nominal_spacing = 1.0;

  /* pass back what Fig will need */
  *style = fig_line_style;
  *spacing = fig_nominal_spacing;
}
