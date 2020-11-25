/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated invocation of the cont() routine.  (See the comments in
   g_cont.c.)  The construction may be terminated, and the polyline object
   finalized, by an explict invocation of endpath().

   If endpath() is invoked when no polyline is under construction, it has
   no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

/* Fig's line styles, indexed into by internal line number
   (L_SOLID/L_DOTTED/ L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED.  Fig format
   supports only dotted lines (isolated dots, with a specified inter-dot
   distance) and dashed lines (on/off dashes, the lengths of the on and off
   segments being equal).  We map our canonical five line types into Fig
   line types as best we can. */
const int _fig_line_style[NUM_LINE_TYPES] =
{ FIG_L_SOLID, FIG_L_DOTTED, FIG_L_DOTTED, FIG_L_DASHED, FIG_L_DASHED };

/* Fig's `style value', i.e. inter-dot length or on-and-off segment length,
   indexed into by internal line number (L_SOLID/L_DOTTED/
   L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED; dash length ignored for
   L_SOLID).  Units are Fig display units.  We'll scale these values by the
   line width (in Fig display units). */
const double _fig_dash_length[NUM_LINE_TYPES] =
{ 0.0, 2.0, 4.0, 4.0, 8.0 };

/* Fig join styles, indexed by internal join type number (miter/rd./bevel) */
const int _fig_join_style[] =
{ FIG_JOIN_MITER, FIG_JOIN_ROUND, FIG_JOIN_BEVEL };

/* Fig cap styles, indexed by internal cap type number (butt/rd./project) */
const int _fig_cap_style[] =
{ FIG_CAP_BUTT, FIG_CAP_ROUND, FIG_CAP_PROJECT };

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

  /* If a circular arc has been stashed rather than drawn, force it to be
     drawn by invoking farc() with the `immediate' flag set.  Note that
     if an arc is stashed, PointsInLine must be zero. */
  if (_plotter->drawstate->arc_stashed) 
    { 
      double axc = _plotter->drawstate->axc;
      double ayc = _plotter->drawstate->ayc;
      double ax0 = _plotter->drawstate->ax0;
      double ay0 = _plotter->drawstate->ay0;
      double ax1 = _plotter->drawstate->ax1;
      double ay1 = _plotter->drawstate->ay1;

      _plotter->drawstate->arc_immediate = true; 
      _plotter->drawstate->arc_polygonal = false; /* advisory only */
      _plotter->farc (axc, ayc, ax0, ay0, ax1, ay1);
      _plotter->drawstate->arc_immediate = false;
      _plotter->drawstate->arc_stashed = false;
    }

  if (_plotter->drawstate->PointsInLine == 0)	/* nothing to do */
    return 0;
  if (_plotter->drawstate->PointsInLine == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;
      return 0;
    }
  
  if ((_plotter->drawstate->PointsInLine >= 3) /* check for closure */
      && (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1].x == _plotter->drawstate->datapoints[0].x)
      && (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1].y == _plotter->drawstate->datapoints[0].y))
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
      Point *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_PointsInLine = _plotter->drawstate->PointsInLine;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;

      _plotter->savestate();
      _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (1);
      _plotter->linewidth (0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_PointsInLine - (closed ? 1 : 0); i++)
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
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_plotter->fig_last_priority >= POLYLINE_PRIORITY)
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;
  _plotter->fig_last_priority = POLYLINE_PRIORITY;

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
	  _plotter->drawstate->PointsInLine	/* number of points in line */
	  );
  _update_buffer (_plotter->page);

  for (i=0; i<_plotter->drawstate->PointsInLine; i++)
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

  /* reset polyline storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->PointsInLine = 0;

  return 0;
}
