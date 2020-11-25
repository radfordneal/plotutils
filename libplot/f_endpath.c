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
#include "plot.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

/* Fig's line styles, indexed into by internal line number
   (L_SOLID/L_DOTTED/ L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED. */
const int _fig_line_style[NUM_LINE_TYPES] =
{ FIG_L_SOLID, FIG_L_DOTTED, FIG_L_DASHDOTTED, FIG_L_DASHED, FIG_L_DASHED };

/* Fig's `style value', i.e. inter-dot length or on-and-off segment length,
   indexed into by internal line number (L_SOLID/L_DOTTED/
   L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED; dash length ignored for
   L_SOLID).  Units are Fig display units.  We'll scale these values by the
   line width (in Fig display units). */
const double _fig_dash_length[NUM_LINE_TYPES] =
{ 0.0, 2.0, 6.0, 4.0, 8.0 };

/* Fig join styles, indexed by internal join type number (miter/rd./bevel) */
const int _fig_join_style[] =
{ FIG_JOIN_MITER, FIG_JOIN_ROUND, FIG_JOIN_BEVEL };

/* Fig cap styles, indexed by internal cap type number (butt/rd./project) */
const int _fig_cap_style[] =
{ FIG_CAP_BUTT, FIG_CAP_ROUND, FIG_CAP_PROJECT };

/* forward references */
static void _f_emit_arc __P((double xc, double yc, double x0, double y0, double x1, double y1));

int
#ifdef _HAVE_PROTOS
_f_endpath (void)
#else
_f_endpath ()
#endif
{
  bool closed; 
  const char *format;
  int i, dash_scale, polyline_subtype;
    
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
  
  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  /* we scale dash length by line width */
  dash_scale = _plotter->drawstate->quantized_device_line_width;
  if (dash_scale == 0)
    dash_scale = 1;

  sprintf(_plotter->page->point,
	  format,
	  2,			/* polyline object */
	  polyline_subtype,	/* polyline subtype */
	  _fig_line_style[_plotter->drawstate->line_type], /* style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
		  /* style val, in Fig display units (float) */
	  dash_scale * _fig_dash_length[_plotter->drawstate->line_type],
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

      if ((i%5) == 0)
	sprintf (_plotter->page->point, "\n\t"); /* make human-readable */
      else
	sprintf (_plotter->page->point, " ");
      _update_buffer (_plotter->page);

      sprintf (_plotter->page->point, 
	       "%d %d", 
	       IROUND(XD((_plotter->drawstate->datapoints)[i].x,
			 (_plotter->drawstate->datapoints)[i].y)),
	       IROUND(YD((_plotter->drawstate->datapoints)[i].x,
			 (_plotter->drawstate->datapoints)[i].y)));
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
   can count on the arc not being of zero length.  See f_arc.c. */

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

static void
#ifdef _HAVE_PROTOS
_f_emit_arc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_f_emit_arc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point p0, p1, pc, pm, pb;
  Vector v, v0, v1;
  double cross, radius;
  int orientation;

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

  pm.x = 0.5 * (p0.x + p1.x);	/* midpoint of chord from p0 to p1 */
  pm.y = 0.5 * (p0.y + p1.y);  

  v.x = p1.x - p0.x;		/* chord vector from p0 to p1 */
  v.y = p1.y - p0.y;
      
  _vscale(&v, radius);
  pb.x = pc.x + orientation * v.y; /* bisection point of arc */
  pb.y = pc.y - orientation * v.x;
      
  /* evaluate fig colors lazily, i.e. only when needed */
  _plotter->set_pen_color();
  _plotter->set_fill_color();
  
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
	  _fig_line_style[_plotter->drawstate->line_type], /* style */
	  			/* thickness, in Fig display units */
	  _plotter->drawstate->quantized_device_line_width, 
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  			/* style val, in Fig display units (float) */
	  _fig_dash_length[_plotter->drawstate->line_type], 
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
