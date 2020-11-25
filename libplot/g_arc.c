/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does.

   This file also contains the ellarc method, which is a GNU extension to
   libplot.  It draws an object: an arc of an ellipse, from p0=(x0,y0) to
   p1=(x1,y1).  The center of the ellipse will be at pc=(xc,yc).

   These conditions do not uniquely determine the elliptic arc (or the
   ellipse of which it is an arc).  We choose the elliptic arc so that it
   has control points p0, p1, and p0 + p1 - pc, where the third control
   point p0 + p1 - pc is simply the reflection of pc through the line
   determined by p0 and p1.  This means that the arc passes through p0 and
   p1, is tangent at p0 to the line segment joining p0 to p0 + p1 - pc, and
   is tangent at p1 to the line segment joining p1 to p0 + p1 - pc.  So it
   fits snugly into a triangle, the vertices of which are the three control
   points.
   
   This sort of elliptic arc is called a `quarter-ellipse', since it is an
   affinely transformed quarter-circle.  Specifically, it is an affinely
   transformed version of the first quadrant of a unit circle, with the
   affine transformation mapping (0,0) to pc, (0,1) to p0, (1,0) to p1, and
   (1,1) to the control point p0 + p1 - pc. */

/* In the arc and ellarc methods below, we either add the arc to our path
   buffer as a single element, or we call _fakearc() to add an inscribed
   polyline to the buffer, by repeatedly invoking fcont().

   _fakearc(), which draws polygonal approximations to arcs, is our basic
   arc-drawing algorithm.  Each polygonal approximation will contain no
   more than 2**MAX_ARC_SUBDIVISIONS line segments, since the subdividing
   stops when MAX_ARC_SUBDIVISIONS have been made.  MAX_ARC_SUBDIVISIONS,
   defined in extern.h, should be no greater than ARC_SUBDIVISIONS, defined
   in g_arc.h as the size of the chordal deviation lookup table.  On a
   raster device, we also stop subdividing when the line segments become
   zero pixels long. */

#include "sys-defines.h"
#include "extern.h"
#include "g_arc.h"

/* initial path buffer size */
#define DATAPOINTS_BUFSIZ MAX_UNFILLED_POLYLINE_LENGTH
#define DIST(p0,p1) (sqrt( ((p0).x - (p1).x)*((p0).x - (p1).x) \
			  + ((p0).y - (p1).y)*((p0).y - (p1).y)))
#define COLLINEAR(p0, p1, p2) \
	((p0.x * p1.y - p0.y * p1.x - p0.x * p2.y + \
	  p0.y * p2.x + p1.x * p2.y - p1.y * p2.x) == 0.0)

/* forward references */
static void _prepare_chord_table ____P ((double sagitta, double custom_chord_table[TABULATED_ARC_SUBDIVISIONS]));

int
#ifdef _HAVE_PROTOS
_g_farc (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
#else
_g_farc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  plPoint p0, p1, pc; 

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter)
		       "farc: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x1,y1).  Only the endpoints will appear on the
     display. */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (R___(_plotter) x0, y0, x1, y1);

  /* Another trivial case: treat a zero-length arc as a line segment */
  if (x0 == x1 && y0 == y1)
    return _plotter->fline (R___(_plotter) x0, y0, x1, y1);

  /* If new arc not contiguous, move to its starting point (thereby ending
     the path under construction, if any, since move() invokes the
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

  /* add new circular arc to the path buffer */

  /* adjust location of pc if necessary, to place it on the bisector */
  p0.x = x0; p0.y = y0;
  p1.x = x1; p1.y = y1;      
  pc.x = xc; pc.y = yc;      
  pc = _truecenter (p0, p1, pc);
  xc = pc.x; yc = pc.y;

  if (((_plotter->have_mixed_paths == false
	&& _plotter->drawstate->points_in_path == 0)
       || _plotter->have_mixed_paths == true)
      && (_plotter->allowed_arc_scaling == AS_ANY
	  || (_plotter->allowed_arc_scaling == AS_UNIFORM
	      && _plotter->drawstate->transform.uniform)
	  || (_plotter->allowed_arc_scaling == AS_AXES_PRESERVED
	      && _plotter->drawstate->transform.axes_preserved)))
      /* add circular arc as an arc element, since it's allowed */
    {
      plGeneralizedPoint newpoint;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress, so begin one (at start of arc) */
	{
	  newpoint.x = x0;
	  newpoint.y = y0;
	  _plotter->drawstate->datapoints[0] = newpoint;
	  _plotter->drawstate->points_in_path++;	  
	}
      
      /* add new generalized point to path buffer, symbolizing circular arc */
      newpoint.type = S_ARC;
      newpoint.x = x1;
      newpoint.y = y1;
      newpoint.xc = xc;
      newpoint.yc = yc;
      _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path++]
	= newpoint;

      _plotter->drawstate->pos.x = x1; /* move to endpoint */
      _plotter->drawstate->pos.y = y1;
    }
  else
    /* add circular arc as a polygonal approximation, by invoking
       _fakearc(), i.e., by invoking fcont() repeatedly */
    _draw_circular_arc (R___(_plotter) p0, p1, pc);

  /* Provided that the Plotter supports the flushing of too-long polylines,
     if the path is getting too long (and it doesn't have to be filled),
     flush it to output and begin a new one.  `Too long' is
     Plotter-dependent.  The `suppress_polyline_flushout' flag is set
     during the drawing of polygonal approximations to ellipses
     (incl. circles), elliptic arcs, and circular arcs.  */
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
_g_fellarc (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
#else
_g_fellarc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  plPoint pc, p0, p1;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter)
		       "fellarc: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x1,y1).  Only the endpoints will appear on the
     display.  */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (R___(_plotter) x0, y0, x1, y1);

  /* Another trivial case: treat a zero-length arc as a line segment */
  if (x0 == x1 && y0 == y1)
    return _plotter->fline (R___(_plotter) x0, y0, x1, y1);

  p0.x = x0; p0.y = y0;
  p1.x = x1; p1.y = y1;      
  pc.x = xc; pc.y = yc;      
  if (COLLINEAR (p0, p1, pc))
    /* collinear points, simply draw line segment from p0 to p1 */
    return _plotter->fline (R___(_plotter) x0, y0, x1, y1);

  /* If new arc not contiguous, move to its starting point (thereby
     finalizing the path under construction, if any, since the move()
     method invokes the endpath() method). */
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

  /* add new elliptic arc to the path buffer */

  if (((_plotter->have_mixed_paths == false
       && _plotter->drawstate->points_in_path == 0)
       || _plotter->have_mixed_paths == true)
      && (_plotter->allowed_ellarc_scaling == AS_ANY
	  || (_plotter->allowed_ellarc_scaling == AS_UNIFORM
	      && _plotter->drawstate->transform.uniform)
	  || (_plotter->allowed_ellarc_scaling == AS_AXES_PRESERVED
	      && _plotter->drawstate->transform.axes_preserved
	      && ((y0 == yc && x1 == xc) || (x0 == xc && y1 == yc)))))
    /* add elliptic arc as an arc element, since it's allowed (note that we
       interpret the AS_AXES_PRESERVED constraint to require also that the
       x and y coors for arc endpoints line up) */
    {
      plGeneralizedPoint newpoint;

      if (_plotter->drawstate->points_in_path == 0)
	/* no path in progress, so begin one (at start of arc) */
	{
	  newpoint.x = x0;
	  newpoint.y = y0;
	  _plotter->drawstate->datapoints[0] = newpoint;
	  _plotter->drawstate->points_in_path++;	  
	}

      /* add new generalized point to path buffer, symbolizing elliptic arc */
      newpoint.type = S_ELLARC;
      newpoint.x = x1;
      newpoint.y = y1;
      newpoint.xc = xc;
      newpoint.yc = yc;
      _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path++]
	= newpoint;

      _plotter->drawstate->pos.x = x1; /* move to endpoint */
      _plotter->drawstate->pos.y = y1;
    }
  else
    /* add elliptic arc as a polygonal approximation, by invoking
       _fakearc(), i.e., by invoking fcont() repeatedly */
    _draw_elliptic_arc (R___(_plotter) p0, p1, pc);

  /* Provided that the Plotter supports the flushing of too-long polylines,
     if the path is getting too long (and it doesn't have to be filled),
     flush it to output and begin a new one.  `Too long' is
     Plotter-dependent.  The `suppress_polyline_flushout' flag is set
     during the drawing of polygonal approximations to ellipses
     (incl. circles), elliptic arcs, and circular arcs.  */
  if (_plotter->flush_long_polylines
      && (_plotter->drawstate->points_in_path 
	  >= _plotter->max_unfilled_polyline_length)
      && !_plotter->drawstate->suppress_polyline_flushout
      && (_plotter->drawstate->fill_type == 0))
    _plotter->endpath (S___(_plotter));
  
  return 0;
}

/* _draw_circular_arc draws a polygonal approximation to the circular arc
   from p0 to p1, with center pc, by calling _fakearc(), which in turn
   invokes fcont() repeatedly.  It is assumed that p0 and p1 are distinct.
   It is assumed that pc is on the perpendicular bisector of the line
   segment joining them, and that the graphics cursor is initially located
   at p0. */
void
#ifdef _HAVE_PROTOS
_draw_circular_arc (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
#else
_draw_circular_arc (R___(_plotter) p0, p1, pc)
     S___(Plotter *_plotter;)
     plPoint p0, p1, pc; 
#endif
{
  /* bisection point of arc, and midpoint of chord */
  plPoint pb, pm;
  /* rotation matrix */
  double m[4];
  /* other variables */
  plVector v, v0, v1;
  double radius, sagitta;
  double cross, orientation;
  bool flushoutp;
  /* handcrafted relative chordal deviation table, for this arc */
  double custom_chord_table[TABULATED_ARC_SUBDIVISIONS];
  
  if (p0.x == p1.x && p0.y == p1.y)
    /* zero-length arc, draw as zero-length line segment */
    _plotter->fcont (R___(_plotter) p0.x, p0.y);

  else
    /* genuine polygonal approximation */
    {
      /* set for duration of this routine; no break in mid-arc allowed */
      flushoutp = _plotter->drawstate->suppress_polyline_flushout;
      _plotter->drawstate->suppress_polyline_flushout = true;

      /* vectors from pc to p0, and pc to p1 */
      v0.x = p0.x - pc.x;
      v0.y = p0.y - pc.y;
      v1.x = p1.x - pc.x;
      v1.y = p1.y - pc.y;
      
      /* cross product, zero if points are collinear */
      cross = v0.x * v1.y - v1.x * v0.y;
      
      /* Compute orientation.  Note libplot convention: if p0, p1, pc are
	 collinear then arc goes counterclockwise from p0 to p1. */
      orientation = (cross >= 0.0 ? 1.0 : -1.0);
      
      radius = DIST(pc, p0);	/* radius is distance to p0 or p1 */
      
      pm.x = 0.5 * (p0.x + p1.x); /* midpoint of chord from p0 to p1 */
      pm.y = 0.5 * (p0.y + p1.y);  
      
      v.x = p1.x - p0.x;	/* chord vector from p0 to p1 */
      v.y = p1.y - p0.y;
      
      _vscale(&v, radius);
      pb.x = pc.x + orientation * v.y; /* bisection point of arc */
      pb.y = pc.y - orientation * v.x;
      
      sagitta = DIST(pb, pm) / radius;
      
      /* fill in entries of chordal deviation table for this user-defined
         sagitta */
      _prepare_chord_table (sagitta, custom_chord_table);
      
      /* call _fakearc(), using for `rotation' matrix m[] a clockwise or
	 counterclockwise rotation by 90 degrees, depending on orientation */
      m[0] = 0.0, m[1] = orientation, m[2] = -orientation, m[3] = 0.0;
      _fakearc (R___(_plotter) 
		p0, p1, USER_DEFINED_ARC, custom_chord_table, m);

      /* reset to original value */
      _plotter->drawstate->suppress_polyline_flushout = flushoutp;
    }

  return;
}

/* _draw_elliptic_arc() draws a polygonal approximation to a
   quarter-ellipse from p0 to p1, by calling _fakearc(), which in turn
   invokes fcont() repeatedly.  For this, it calls _fakearc(), preserving
   drawing attributes.  pc is the center of the arc, and p0, p1, pc are
   assumed not to be collinear.  It is assumed that the graphics cursor is
   located at p0 when this function is called.

   The control triangle for the elliptic arc will have vertices p0, p1, and
   K = p0 + (p1 - pc) = p1 + (p0 - pc).  The arc will pass through p0 and
   p1, and will be tangent at p0 to the edge from p0 to K, and at p1 to the
   edge from p1 to K. */
void 
#ifdef _HAVE_PROTOS
_draw_elliptic_arc (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
#else
_draw_elliptic_arc (R___(_plotter) p0, p1, pc)
     S___(Plotter *_plotter;)
     plPoint p0, p1, pc; 
#endif
{ 
  plVector v0, v1; 
  double cross;
  double m[4];
  bool flushoutp;

  /* set for duration of this routine; no break in mid-arc allowed */
  flushoutp = _plotter->drawstate->suppress_polyline_flushout;
  _plotter->drawstate->suppress_polyline_flushout = true;

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

  /* cross product */
  cross = v0.x * v1.y - v1.x * v0.y;
  if (cross == 0.0)
    /* collinear points, draw line segment from p0 to p1
       (not quite right, could be bettered) */
    _plotter->fcont (R___(_plotter) p1.x, p1.y);
  else
    {
      /* `rotation' matrix (it maps v0 -> -v1 and v1 -> v0) */
      m[0] = - (v0.x * v0.y + v1.x * v1.y) / cross;
      m[1] = (v0.x * v0.x + v1.x * v1.x) / cross;
      m[2] = - (v0.y * v0.y + v1.y * v1.y) / cross;
      m[3] = (v0.x * v0.y + v1.x * v1.y) / cross;
      
      /* draw polyline inscribed in the quarter-ellipse */
      _fakearc (R___(_plotter) p0, p1, QUARTER_ARC, (double *)NULL, m);
    }

  /* reset to original value */
  _plotter->drawstate->suppress_polyline_flushout = flushoutp;

  return;
}

/* The _fakearc() subroutine below contains our arc subdivision algorithm,
   a remote descendent of the arc-drawing algorithm of Ken Turkowski
   <turk@apple.com> described in Graphics Gems V.  His algorithm is a
   recursive circle subdivision algorithm, which relies on the fact that if
   s and s' are the (chordal deviation)/radius associated to (respectively)
   an arc and a half-arc, then s' is approximately equal to s/4.  The exact
   formula is s' = 1 - sqrt (1 - s/2), which applies for all s in the
   meaningful range, i.e. 0 <= s <= 2.

   Ken's algorithm rotates the chord of an arc by 90 degrees and scales it
   to have length s'.  The resulting vector will be the chordal deviation
   vector of the arc, which gives the midpoint of the arc, and hence the
   endpoints of each half of the arc.  So this drawing technique is
   recursive.

   The problem with this approach is that scaling a vector to a specified
   length requires a square root, so there are two square roots in each
   subdivision step.  We could attempt to remove one of them by noticing
   that the chord half-length h always satisfies h = sqrt(s * (2-s))).  So
   we can rotate the chord vector by 90 degrees, and multiply its length by
   s/2h, i.e., s/2sqrt(s * (2-s)), to get the chordal deviation vector.
   This factor still includes a square root though.  Also we would still
   need to compute a square root to proceed from one subdivision step to
   the next, i.e. to compute s' from s.

   We get around the square root problem by drawing only circular arcs with
   subtended angle of 90 degrees (quarter-circles), or elliptic arcs that
   are obtained from such quarter-circles by affine transformations
   (so-called quarter-ellipses).  To draw the latter, we need only replace
   the 90 degree rotation matrix mentioned above by an affine
   transformation that maps v0->-v1 and v1->v0, where v0 = p0 - pc and v1 =
   p1 - pc are the displacement vectors from the center of the ellipse to
   the endpoints of the arc.  If we do this, we get an elliptic arc with p0
   and p1 as endpoints. The vectors v0 and v1 are said to lie along
   conjugate diameters of the quarter-ellipse.
   
   So for drawing quarter-ellipses, the only initial value of s we need to
   consider is the one for a quarter-circle, which is 1-sqrt(1/2).  The
   successive values of s/2h that will be encountered, after each
   bisection, are pre-computed and stored in a lookup table (see g_arc.h).

   This approach lets us avoid, completely, any computation of square roots
   during the drawing of quarter-circles and quarter-ellipses.  The only
   square roots we need are precomputed.  We don't need any floating point
   divisions in the main loop either.  

   The implementation below does not use recursion (we use a local array,
   not the call stack, to store the sequence of generated points). 

   Note: the argument arc_type of _fakearc() can be set to QUARTER_ARC,
   HALF_ARC, or THREE_QUARTER_ARC, i.e., 0, 1, or 2.  The chordal deviation
   table in g_arc.h contains sub-tables for each of these three cases.  If
   arc_type is USER_DEFINED_ARC, the caller must supply a lookup table. */

/* definition of `same point' in the user frame, that takes into account
   whether or not the device coordinates we'll use are integer-valued */
#define SAME_POINT(p0, p1) (_plotter->display_coors_type != (int)DISP_DEVICE_COORS_REAL ? \
			     ((IROUND(XD((p0).x, (p0).y)) \
			      == IROUND(XD((p1).x, (p1).y))) \
			     && (IROUND(YD((p0).x,(p0).y)) \
				 == IROUND(YD((p1).x,(p1).y)))) \
			    : ((p0).x == (p1).x && (p0).y == (p1).y))

void 
#ifdef _HAVE_PROTOS
_fakearc(R___(Plotter *_plotter) plPoint p0, plPoint p1, int arc_type, const double *custom_chord_table, const double m[4])
#else
_fakearc(R___(_plotter) p0, p1, arc_type, custom_chord_table, m)
     S___(Plotter *_plotter;)
     plPoint p0, p1;
     int arc_type;  /* {QUARTER,HALF,THREE_QUARTER,USER_DEFINED}_ARC */
     const double *custom_chord_table; /* user-supplied lookup table */
     const double m[4];
#endif
{
  plPoint p[MAX_ARC_SUBDIVISIONS + 1], q[MAX_ARC_SUBDIVISIONS + 1];
  int level[MAX_ARC_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < MAX_ARC_SUBDIVISIONS */
  int segments_drawn = 0;
  const double *our_chord_table;

  if (arc_type == USER_DEFINED_ARC)
    our_chord_table = custom_chord_table;
  else				/* custom_chord_table arg ignored */
    our_chord_table = _chord_table[arc_type];

  p[0] = p0;
  q[0] = p1;
  level[0] = 0;
  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      if (SAME_POINT(p[n], q[n]))
	n--;			/* discard zero-length line segment */
      
      else if (level[n] >= MAX_ARC_SUBDIVISIONS) 
	{			/* draw line segment */
	  _plotter->fcont (R___(_plotter) q[n].x, q[n].y);
	  segments_drawn++;
	  n--;
	}
      
      else			/* bisect line segment */
	{
	  plVector v;
	  plPoint pm, pb;
	  
	  v.x = q[n].x - p[n].x; /* chord = line segment from p[n] to q[n] */
	  v.y = q[n].y - p[n].y;
	  
	  pm.x = p[n].x + 0.5 * v.x; /* midpoint of chord */
	  pm.y = p[n].y + 0.5 * v.y;
	  
	  /* Compute bisection point.  If m=[0 1 -1 0] this just rotates
	     the chord clockwise by 90 degrees, and scales it to yield the
	     chordal deviation vector, which is used as an offset. */
	  
	  pb.x = pm.x + 
	    our_chord_table[level[n]] * (m[0] * v.x + m[1] * v.y);
	  pb.y = pm.y + 
	    our_chord_table[level[n]] * (m[2] * v.x + m[3] * v.y);
	  
	  /* replace line segment by pair; level[n] >= n is an invariant */
	  p[n+1] = p[n];
	  q[n+1] = pb;		/* first half, deal with next */
	  level[n+1] = level[n] + 1;
	  
	  p[n] = pb;
	  q[n] = q[n];		/* second half, deal with later */
	  level[n] = level[n] + 1;

	  n++;
	}
    }

  /* be sure to draw at least one line segment; also, if due to the line
     segments getting too short we stopped before we reached p1, draw a
     final line segment */
  if (segments_drawn == 0
      || (_plotter->drawstate->pos.x != q[0].x
	  || _plotter->drawstate->pos.y != q[0].y))
    _plotter->fcont (R___(_plotter) q[0].x, q[0].y);

  return;
}

/* prepare_chord_table() computes the list of chordal deviation factors
   that _fakearc() needs when it is employed to draw a circular arc of
   subtended angle other than the default angles it supports */
static void
#ifdef _HAVE_PROTOS
_prepare_chord_table (double sagitta, double custom_chord_table[TABULATED_ARC_SUBDIVISIONS])
#else
_prepare_chord_table (sagitta, custom_chord_table)
     double sagitta;
     double custom_chord_table[TABULATED_ARC_SUBDIVISIONS];
#endif
{
  double half_chord_length;
  int i;

  half_chord_length = sqrt ( sagitta * (2.0 - sagitta) );
  for (i = 0; i < TABULATED_ARC_SUBDIVISIONS; i++)
    {
      custom_chord_table[i] = 0.5 * sagitta / half_chord_length;
      sagitta = 1.0 - sqrt (1.0 - 0.5 * sagitta);
      half_chord_length = 0.5 * half_chord_length / (1.0 - sagitta);
    }

  return;
}
