/* This file contains the bezier2 and bezier3 methods, which are GNU
   extensions to libplot.  Each of them draws an object: a quadratic and a
   cubic Bezier path segment, respectively. */

#include "sys-defines.h"
#include "extern.h"

/* max number of times a Bezier will be subdivided, when constructing a
   polygonal approximation */
#define MAX_BEZIER_SUBDIVISIONS 7

#define DATAPOINTS_BUFSIZ MAX_UNFILLED_POLYLINE_LENGTH

int
#ifdef _HAVE_PROTOS
_g_fbezier2 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2)
#else
_g_fbezier2 (R___(_plotter) x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1, x2, y2;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbezier2: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x2,y2).  Only the endpoints will appear on the
     display. */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (R___(_plotter) x0, y0, x2, y2);

  /* If new segment not contiguous, move to its starting point (thereby
     ending the path under construction, if any, since move() invokes the
     endpath() method). */
  if (x0 != _plotter->drawstate->pos.x 
      || y0 != _plotter->drawstate->pos.y)
    _plotter->fmove (R___(_plotter) x0, y0);
  
  /* if path buffer exists and is occupied by a single arc, replace arc by
     a polyline if that's called for */
  if (_plotter->have_mixed_paths == false
      && _plotter->drawstate->points_in_path == 2)
    _maybe_replace_arc (S___(_plotter));

  /* create or adjust size of path buffer, as needed */
  if (_plotter->drawstate->datapoints_len == 0)
    {
      _plotter->drawstate->datapoints = (plGeneralizedPoint *) 
	_plot_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plGeneralizedPoint));
      _plotter->drawstate->datapoints_len = DATAPOINTS_BUFSIZ;
    }
  if (_plotter->drawstate->points_in_path == _plotter->drawstate->datapoints_len)
    {
      _plotter->drawstate->datapoints = (plGeneralizedPoint *) 
	_plot_xrealloc (_plotter->drawstate->datapoints, 
			2 * _plotter->drawstate->datapoints_len * sizeof(plGeneralizedPoint));
      _plotter->drawstate->datapoints_len *= 2;
    }

  /* add new quadratic Bezier segment to the path buffer */

  if (_plotter->allowed_quad_scaling == AS_ANY)
    /* add as a primitive element, since it's allowed */
    {
      plGeneralizedPoint newpoint;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress, so begin one (at start of segment) */
	{
	  newpoint.x = x0;
	  newpoint.y = y0;
	  _plotter->drawstate->datapoints[0] = newpoint;
	  _plotter->drawstate->points_in_path++;	  
	}
      
      /* add new generalized point, symbolizing Bezier, to path buffer */
      newpoint.type = S_QUAD;
      newpoint.x = x2;
      newpoint.y = y2;
      newpoint.xc = x1;
      newpoint.yc = y1;
      _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path++]
	= newpoint;

      _plotter->drawstate->pos.x = x2; /* move to endpoint */
      _plotter->drawstate->pos.y = y2;
    }

  else if (_plotter->allowed_cubic_scaling == AS_ANY)
    /* add quadratic Bezier as a cubic Bezier, since it's allowed */
    {
      plGeneralizedPoint newpoint;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress, so begin one (at start of segment) */
	{
	  newpoint.x = x0;
	  newpoint.y = y0;
	  _plotter->drawstate->datapoints[0] = newpoint;
	  _plotter->drawstate->points_in_path++;	  
	}
      
      /* add new generalized point, symbolizing cubic Bezier, to path buffer
	 (control points need to be computed) */
      newpoint.type = S_CUBIC;
      newpoint.x = x2;
      newpoint.y = y2;
      newpoint.xc = (2.0 * x1 + x0) / 3.0;
      newpoint.yc = (2.0 * y1 + y0) / 3.0;
      newpoint.xd = (2.0 * x1 + x2) / 3.0;
      newpoint.yd = (2.0 * y1 + y2) / 3.0;
      _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path++]
	= newpoint;

      _plotter->drawstate->pos.x = x2; /* move to endpoint */
      _plotter->drawstate->pos.y = y2;
    }
  else
    /* add quadratic Bezier segment as a polygonal approximation, by
       invoking fcont() repeatedly */
    {
      plPoint p0, p1, p2; 

      p0.x = x0; p0.y = y0;
      p1.x = x1; p1.y = y1;      
      p2.x = x2; p2.y = y2;      
      _draw_bezier2 (R___(_plotter) p0, p1, p2);
    }

  /* Provided that the Plotter supports the flushing of too-long polylines,
     if the path is getting too long (and it doesn't have to be filled),
     flush it to output and begin a new one.  `Too long' is
     Plotter-dependent. */
  if (_plotter->flush_long_polylines
      && (_plotter->drawstate->points_in_path 
	  >= _plotter->max_unfilled_polyline_length)
      && !_plotter->drawstate->suppress_polyline_flushout
      && (_plotter->drawstate->fill_type == 0))
    _plotter->endpath (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_fbezier3 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
_g_fbezier3 (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;)
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbezier3: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x3,y3).  Only the endpoints will appear on the
     display. */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (R___(_plotter) x0, y0, x3, y3);

  /* If new segment not contiguous, move to its starting point (thereby
     ending the path under construction, if any, since move() invokes the
     endpath() method). */
  if (x0 != _plotter->drawstate->pos.x 
      || y0 != _plotter->drawstate->pos.y)
    _plotter->fmove (R___(_plotter) x0, y0);
  
  /* if path buffer exists and is occupied by a single arc, replace arc by
     a polyline if that's called for */
  if (_plotter->have_mixed_paths == false
      && _plotter->drawstate->points_in_path == 2)
    _maybe_replace_arc (S___(_plotter));

  /* create or adjust size of path buffer, as needed */
  if (_plotter->drawstate->datapoints_len == 0)
    {
      _plotter->drawstate->datapoints = (plGeneralizedPoint *) 
	_plot_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plGeneralizedPoint));
      _plotter->drawstate->datapoints_len = DATAPOINTS_BUFSIZ;
    }
  if (_plotter->drawstate->points_in_path == _plotter->drawstate->datapoints_len)
    {
      _plotter->drawstate->datapoints = (plGeneralizedPoint *) 
	_plot_xrealloc (_plotter->drawstate->datapoints, 
			2 * _plotter->drawstate->datapoints_len * sizeof(plGeneralizedPoint));
      _plotter->drawstate->datapoints_len *= 2;
    }

  /* add new cubic Bezier segment to the path buffer */

  if (_plotter->allowed_cubic_scaling == AS_ANY)
    /* add as a primitive element, since it's allowed */
    {
      plGeneralizedPoint newpoint;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress, so begin one (at start of segment) */
	{
	  newpoint.x = x0;
	  newpoint.y = y0;
	  _plotter->drawstate->datapoints[0] = newpoint;
	  _plotter->drawstate->points_in_path++;	  
	}
      
      /* add new generalized point, symbolizing Bezier, to path buffer */
      newpoint.type = S_CUBIC;
      newpoint.x = x3;
      newpoint.y = y3;
      newpoint.xc = x1;
      newpoint.yc = y1;
      newpoint.xd = x2;
      newpoint.yd = y2;
      _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path++]
	= newpoint;

      _plotter->drawstate->pos.x = x3; /* move to endpoint */
      _plotter->drawstate->pos.y = y3;
    }
  else
    /* add quadratic Bezier segment as a polygonal approximation, by
       invoking fcont() repeatedly */
    {
      plPoint p0, p1, p2, p3;

      p0.x = x0; p0.y = y0;
      p1.x = x1; p1.y = y1;      
      p2.x = x2; p2.y = y2;      
      p3.x = x3; p3.y = y3;      
      _draw_bezier3 (R___(_plotter) p0, p1, p2, p3);
    }

  /* Provided that the Plotter supports the flushing of too-long polylines,
     if the path is getting too long (and it doesn't have to be filled),
     flush it to output and begin a new one.  `Too long' is
     Plotter-dependent. */
  if (_plotter->flush_long_polylines
      && (_plotter->drawstate->points_in_path 
	  >= _plotter->max_unfilled_polyline_length)
      && !_plotter->drawstate->suppress_polyline_flushout
      && (_plotter->drawstate->fill_type == 0))
    _plotter->endpath (S___(_plotter));
  
  return 0;
}

void 
#ifdef _HAVE_PROTOS
_draw_bezier2 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint p2)
#else
_draw_bezier2 (R___(_plotter) p0, p1, p2)
     S___(Plotter *_plotter;) 
     plPoint p0, p1, p2; 
#endif
{ 
  bool flushoutp;

  /* set for duration of this routine; no break in mid-arc allowed */
  flushoutp = _plotter->drawstate->suppress_polyline_flushout;
  _plotter->drawstate->suppress_polyline_flushout = true;

  /* draw inscribed polyline */
  _fakebezier2 (R___(_plotter) p0, p1, p2);

  /* reset to original value */
  _plotter->drawstate->suppress_polyline_flushout = flushoutp;
}

void 
#ifdef _HAVE_PROTOS
_draw_bezier3 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint p2, plPoint p3)
#else
_draw_bezier3 (R___(_plotter) p0, p1, p2, p3)
     S___(Plotter *_plotter;) 
     plPoint p0, p1, p2, p3; 
#endif
{ 
  bool flushoutp;

  /* set for duration of this routine; no break in mid-arc allowed */
  flushoutp = _plotter->drawstate->suppress_polyline_flushout;
  _plotter->drawstate->suppress_polyline_flushout = true;

  /* draw inscribed polyline */
  _fakebezier3 (R___(_plotter) p0, p1, p2, p3);

  /* reset to original value */
  _plotter->drawstate->suppress_polyline_flushout = flushoutp;
}

/* definition of `same point' in the user frame, that takes into account
   whether or not the device coordinates we'll use are integer-valued */
#define SAME_POINT(p0, p1) (_plotter->display_coors_type != (int)DISP_DEVICE_COORS_REAL ? \
			     ((IROUND(XD((p0).x, (p0).y)) \
			      == IROUND(XD((p1).x, (p1).y))) \
			     && (IROUND(YD((p0).x,(p0).y)) \
				 == IROUND(YD((p1).x,(p1).y)))) \
			    : ((p0).x == (p1).x && (p0).y == (p1).y))

#define MIDWAY(x0, x1) (0.5 * ((x0) + (x1)))

void 
#ifdef _HAVE_PROTOS
_fakebezier2 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint p2)
#else
_fakebezier2 (R___(_plotter) p0, p1, p2)
     S___(Plotter *_plotter;) 
     plPoint p0, p1, p2; 
#endif
{ 
  plPoint r0[MAX_BEZIER_SUBDIVISIONS + 1], r1[MAX_BEZIER_SUBDIVISIONS + 1], r2[MAX_BEZIER_SUBDIVISIONS + 1];
  int level[MAX_BEZIER_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < MAX_BEZIER_SUBDIVISIONS */
  int segments_drawn = 0;

  r0[0] = p0;
  r1[0] = p1;
  r2[0] = p2;
  level[0] = 0;
  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      int current_level;
      plPoint q0, q1, q2;

      current_level = level[n];
      q0 = r0[n];
      q1 = r1[n];
      q2 = r2[n];

      if (SAME_POINT(q0, q1) && SAME_POINT(q1, q2))
	n--;			/* discard zero-length segment */
      
      else if (current_level >= MAX_BEZIER_SUBDIVISIONS) 
	{			/* draw line segment */
	  _plotter->fcont (R___(_plotter) q2.x, q2.y);
	  segments_drawn++;
	  n--;
	}
      
      else			/* bisect the quadratic Bezier */
	{
	  plPoint qq0, qq1;
	  plPoint qqq0;

	  qq0.x = MIDWAY(q0.x, q1.x);
	  qq0.y = MIDWAY(q0.y, q1.y);
	  qq1.x = MIDWAY(q1.x, q2.x);
	  qq1.y = MIDWAY(q1.y, q2.y);

	  qqq0.x = MIDWAY(qq0.x, qq1.x);
	  qqq0.y = MIDWAY(qq0.y, qq1.y);

	  /* replace Bezier by pair; level[n] >= n is an invariant */
	  /* first half, deal with next */
	  r0[n+1] = q0;
	  r1[n+1] = qq0;
	  r2[n+1] = qqq0;
	  level[n+1] = current_level + 1;
	  
	  /* second half, deal with later */
	  r0[n] = qqq0;
	  r1[n] = qq1;
	  r2[n] = q2;		/* i.e. unchanged */
	  level[n] = current_level + 1;

	  n++;
	}
    }

  /* be sure to draw at least one line segment; also, if due to the line
     segments getting too short we stopped before we reached p2, draw a
     final line segment */
  if (segments_drawn == 0
      || (_plotter->drawstate->pos.x != r2[0].x
	  || _plotter->drawstate->pos.y != r2[0].y))
    _plotter->fcont (R___(_plotter) r2[0].x, r2[0].y);
}

void 
#ifdef _HAVE_PROTOS
_fakebezier3 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint p2, plPoint p3)
#else
_fakebezier3 (R___(_plotter) p0, p1, p2, p3)
     S___(Plotter *_plotter;)
     plPoint p0, p1, p2, p3; 
#endif
{ 
  plPoint r0[MAX_BEZIER_SUBDIVISIONS + 1], r1[MAX_BEZIER_SUBDIVISIONS + 1], r2[MAX_BEZIER_SUBDIVISIONS + 1], r3[MAX_BEZIER_SUBDIVISIONS + 1];
  int level[MAX_BEZIER_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < MAX_BEZIER_SUBDIVISIONS */
  int segments_drawn = 0;

  r0[0] = p0;
  r1[0] = p1;
  r2[0] = p2;
  r3[0] = p3;
  level[0] = 0;

  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      int current_level;
      plPoint q0, q1, q2, q3;

      current_level = level[n];
      q0 = r0[n];
      q1 = r1[n];
      q2 = r2[n];
      q3 = r3[n];

      if (SAME_POINT(q0, q1) && SAME_POINT(q1, q2) && SAME_POINT(q2, q3))
	n--;			/* discard zero-length segment */
      
      else if (current_level >= MAX_BEZIER_SUBDIVISIONS) 
	{			/* draw line segment */
	  _plotter->fcont (R___(_plotter) q3.x, q3.y);
	  segments_drawn++;
	  n--;
	}
      
      else			/* bisect the cubic Bezier */
	{
	  plPoint qq0, qq1, qq2;
	  plPoint qqq0, qqq1;
	  plPoint qqqq0;

	  qq0.x = MIDWAY(q0.x, q1.x);
	  qq0.y = MIDWAY(q0.y, q1.y);
	  qq1.x = MIDWAY(q1.x, q2.x);
	  qq1.y = MIDWAY(q1.y, q2.y);
	  qq2.x = MIDWAY(q2.x, q3.x);
	  qq2.y = MIDWAY(q2.y, q3.y);

	  qqq0.x = MIDWAY(qq0.x, qq1.x);
	  qqq0.y = MIDWAY(qq0.y, qq1.y);
	  qqq1.x = MIDWAY(qq1.x, qq2.x);
	  qqq1.y = MIDWAY(qq1.y, qq2.y);

	  qqqq0.x = MIDWAY(qqq0.x, qqq1.x);
	  qqqq0.y = MIDWAY(qqq0.y, qqq1.y);

	  /* replace Bezier by pair; level[n] >= n is an invariant */
	  /* first half, deal with next */
	  level[n+1] = current_level + 1;
	  r0[n+1] = q0;
	  r1[n+1] = qq0;
	  r2[n+1] = qqq0;
	  r3[n+1] = qqqq0;
	  
	  /* second half, deal with later */
	  level[n] = current_level + 1;
	  r0[n] = qqqq0;
	  r1[n] = qqq1;
	  r2[n] = qq2;
	  r3[n] = q3;		/* i.e. unchanged */

	  n++;
	}
    }

  /* be sure to draw at least one line segment; also, if due to the line
     segments getting too short we stopped before we reached p3, draw a
     final line segment */
  if (segments_drawn == 0
      || (_plotter->drawstate->pos.x != r3[0].x
	  || _plotter->drawstate->pos.y != r3[0].y))
    _plotter->fcont (R___(_plotter) r3[0].x, r3[0].y);
}
