/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does. */

/* This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the
   x-axis). */

/* This file contains as well the ellarc method, which is a GNU extension
   to libplot.  It draws an object: an arc of an ellipse, from p0=(x0,y0)
   to p1=(x1,y1).  The center of the ellipse will be at pc=(xc,yc).

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

/* In the generic versions below, we call _fakearc() to draw inscribed
   polylines as approximations, using move() and cont().  For an ellipse,
   polygonal approximations to two half-ellipses are drawn in sequence. */

/* Each inscribed polyline will contain no more than
   2**MAX_ARC_SUBDIVISIONS line segments, since the subdividing stops when
   MAX_ARC_SUBDIVISIONS have been made.  MAX_ARC_SUBDIVISIONS, defined in
   extern.h, should be no greater than ARC_SUBDIVISIONS, defined in
   fakearc.h as the size of a lookup table.  On a raster device, we also
   stop subdividing when the line segments become zero pixels long. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_fakearc.h"

#define DIST(p0,p1) (sqrt( ((p0).x - (p1).x)*((p0).x - (p1).x) \
			  + ((p0).y - (p1).y)*((p0).y - (p1).y)))

/* forward references */
static void _draw_ellipse __P((double xc, double yc, double rx, double ry, double angle));
static void _draw_elliptic_arc __P((Point p0, Point p1, Point pc));
static void _fakearc __P ((Point p0, Point p1, int arc_type, const double m[4]));
static void _prepare_chord_table __P ((double sagitta));

int
#ifdef _HAVE_PROTOS
_g_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_g_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point pc, p0, p1;

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  if (!_plotter->open)
    {
      _plotter->error ("farc: invalid operation");
      return -1;
    }

  /* Non-immediate case: an arc was previously stashed rather than drawn,
     so flush it out by drawing a polygonal approximation to it.  We
     arrange this by setting the `immediate' flag. */
  if (!_plotter->drawstate->arc_immediate 
      && _plotter->drawstate->arc_stashed)
    {
      double axc = _plotter->drawstate->axc;
      double ayc = _plotter->drawstate->ayc;
      double ax0 = _plotter->drawstate->ax0;
      double ay0 = _plotter->drawstate->ay0;
      double ax1 = _plotter->drawstate->ax1;
      double ay1 = _plotter->drawstate->ay1;

      _plotter->drawstate->arc_immediate = true;
      _plotter->drawstate->arc_polygonal = true;
      _plotter->farc (axc, ayc, ax0, ay0, ax1, ay1);
      _plotter->drawstate->arc_immediate = false;
      _plotter->drawstate->arc_stashed = false;
    }

  /* Non-immediate case: if new arc not contiguous, move to its starting
     point (thereby finalizing the path under construction, if any, since
     the move() method invokes the endpath() method). */
  if ((!_plotter->drawstate->arc_immediate)
      && (x0 != _plotter->drawstate->pos.x 
	  || y0 != _plotter->drawstate->pos.y))
    _plotter->fmove (x0, y0);
  
  if (!_plotter->drawstate->arc_immediate
      && _plotter->drawstate->PointsInLine == 0)
    /* Non-immediate case, with no polyline under construction:
       don't actually draw the arc, just stash it for later drawing */
    {
      _plotter->drawstate->axc = xc;
      _plotter->drawstate->ayc = yc;      
      _plotter->drawstate->ax0 = x0;
      _plotter->drawstate->ay0 = y0;      
      _plotter->drawstate->ax1 = x1;
      _plotter->drawstate->ay1 = y1;      
      _plotter->drawstate->arc_stashed = true;
      return 0;
    }
  else
    /* Immediate case, or non-immediate case with a polyline under
       construction: draw a polygonal approximation to the arc by invoking
       fcont() repeatedly.  We don't look at the advisory `arc_polygonal'
       flag in the immediate case, because in this generic method,
       polygonal approximations to arcs are the only kinds of arcs drawn */
    {
      _plotter->drawstate->arc_stashed = false;	/* avoid infinite recursion */
      _draw_circular_arc (p0, p1, pc);
    }

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_fellarc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_g_fellarc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point pc, p0, p1;

  if (!_plotter->open)
    {
      _plotter->error ("fellarc: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x1,y1).  Only the endpoints will appear on the
     display.  */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (x0, y0, x1, y1);

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;  
  p1.x = x1, p1.y = y1;

  /* If new arc not contiguous, move to its starting point (thereby
     finalizing the path under construction, if any, since the move()
     method invokes the endpath() method). */
  if (x0 != _plotter->drawstate->pos.x
      || y0 != _plotter->drawstate->pos.y)
    _plotter->fmove (x0, y0);

  /* call fcont() repeatedly, to draw an inscribed polyline */
  _draw_elliptic_arc (p0, p1, pc);

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_fellipse (double xc, double yc, double rx, double ry, double angle)
#else
_g_fellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fellipse: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->PointsInLine > 0)
    _plotter->endpath(); /* flush polyline if any */

  _draw_ellipse (xc, yc, rx, ry, angle);

  _plotter->fmove (xc, yc);	/* move to center (a libplot convention) */

  return 0;
}
/* _draw_circular_arc draws a polygonal approximation to the circular arc
   from p0 to p1, with center pc, by invoking cont() repeatedly.  It is
   assumed that p0 and p1 are distinct.  If pc is not on the perpendicular
   bisector of the line segment joining them, it is adjusted so that it is.
   When this function is called, it is assumed that the graphics cursor is
   located at p0. */
void
#ifdef _HAVE_PROTOS
_draw_circular_arc(Point p0, Point p1, Point pc)
#else
_draw_circular_arc(p0, p1, pc)
     Point p0, p1, pc; 
#endif
{
  /* bisection point of arc, and midpoint of chord */
  Point pb, pm;
  /* rotation matrix */
  double m[4];
  /* other variables */
  Vector v, v0, v1;
  double radius, sagitta;
  double cross, orientation;
  bool flushoutp;
  
  if (p0.x == p1.x && p0.y == p1.y)
    /* zero-length arc */
    _plotter->fcont (p0.x, p0.y);

  else
    /* genuine polygonal approximation */
    {
      /* set for duration of this routine; no break in mid-arc allowed */
      flushoutp = _plotter->drawstate->suppress_polyline_flushout;
      _plotter->drawstate->suppress_polyline_flushout = true;

      /* adjust location of pc */
      pc = _truecenter (p0, p1, pc);
      
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
      
      /* fill in entries of chord factor table for this user defined
         sagitta */
      _prepare_chord_table (sagitta);
      
      /* call _fakearc(), using for `rotation' matrix m[] a clockwise or
	 counterclockwise rotation by 90 degrees, depending on orientation */
      m[0] = 0.0, m[1] = orientation, m[2] = -orientation, m[3] = 0.0;
      _fakearc (p0, p1, USER_DEFINED_ARC, m);

      /* reset to original value */
      _plotter->drawstate->suppress_polyline_flushout = flushoutp;
    }

  return;
}

/* _draw_elliptic_arc() draws a polygonal approximation to a
   quarter-ellipse from p0 to p1, by invoking cont() repeatedly.  For this,
   it calls _fakearc(), preserving drawing attributes.  pc is the center of
   the arc, and p0, p1, pc are assumed not to be collinear.  It is assumed
   that the graphics cursor is located at p0 when this function is called.

   The control triangle for the elliptic arc will have vertices p0, p1, and
   K = p0 + (p1 - pc) = p1 + (p0 - pc).  The arc will pass through p0 and
   p1, and will be tangent at p0 to the edge from p0 to K, and at p1 to the
   edge from p1 to K. */
static void 
#ifdef _HAVE_PROTOS
_draw_elliptic_arc (Point p0, Point p1, Point pc)
#else
_draw_elliptic_arc (p0, p1, pc)
     Point p0, p1, pc; 
#endif
{ 
  Vector v0, v1; 
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
    _plotter->fcont (p1.x, p1.y);
  else
    {
      /* `rotation' matrix (it maps v0 -> -v1 and v1 -> v0) */
      m[0] = - (v0.x * v0.y + v1.x * v1.y) / cross;
      m[1] = (v0.x * v0.x + v1.x * v1.x) / cross;
      m[2] = - (v0.y * v0.y + v1.y * v1.y) / cross;
      m[3] = (v0.x * v0.y + v1.x * v1.y) / cross;
      
      /* draw inscribed polyline */
      _fakearc (p0, p1, QUARTER_ARC, m);
    }

  /* reset to original value */
  _plotter->drawstate->suppress_polyline_flushout = flushoutp;

  return;
}

/* _draw_ellipse() draws an inscribed polyline approximating an ellipse,
   with center (xc,yc) and semi-axes of length rx and ry (the former
   inclined at a specified angle to the x-axis).  The join mode saved and
   restored (it is changed to `round', to improve smoothness).

   This calls _fakearc() twice, to draw polygonal approximations to two
   semi-ellipses.  If v0 = rx (cos theta, sin theta) and 
   v1 = ry (-sin theta, cos theta) are the two semi-axes of the ellipse, 
   the semi-ellipses go counterclockwise from p0 = pc + v0 to p1 = pc + v1,
   and back. */
static void
#ifdef _HAVE_PROTOS
_draw_ellipse (double xc, double yc, double rx, double ry, double angle)
#else
_draw_ellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
#endif
{
  Point p0, p1;

  double s = sin(M_PI * angle / 180.0);
  double c = cos(M_PI * angle / 180.0);  
  double m[4];
  char *old_join_mode;
  bool flushoutp;
  
  /* save current values of relevant drawing attribute */
  old_join_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->join_mode) + 1);
  strcpy (old_join_mode, _plotter->drawstate->join_mode);
  _plotter->joinmod ("round");	/* set while drawing, for smoothness */
  
  /* set for duration of this routine; no break in mid-arc allowed */
  flushoutp = _plotter->drawstate->suppress_polyline_flushout;
  _plotter->drawstate->suppress_polyline_flushout = true;

  if ((rx == 0.0) || (ry == 0.0)) /* trivial (degenerate) case */
    {
      double dx = 0.0, dy = 0.0;

      if (ry == 0.0)
	{
	  dx = rx * c;
	  dy = rx * s;
	}
      else if (rx == 0.0)
	{
	  dx = -ry * s;
	  dy = ry * c;
	}

      fmove (xc + dx, yc + dy);
      fcont (xc - dx, yc - dy);
      fcont (xc + dx, yc + dy);
    }
  else		/* semiaxes both nonzero, draw a nontrivial polygonal arc */
    {
      p0.x = xc + rx * c;
      p0.y = yc + rx * s;
      p1.x = xc - rx * c;
      p1.y = yc - rx * s;
      
      /* This `rotation' matrix, which reduces to the 90-degree rotation
	 matrix [0 1 -1 0] when rx=ry and angle=0, is what we must use to
	 draw a semiellipse.  If v0 = rx (cos theta, sin theta)
	 and v1 = ry (-sin theta, cos theta) are the two semi-axes of
	 the ellipse, then this matrix maps v0 -> -v1 and v1 -> v0. */
      m[0] = ((ry / rx) - (rx / ry)) * s * c;
      m[1] = (ry / rx) * s * s + (rx / ry) * c * c;
      m[2] = -(rx / ry) * s * s - (ry / rx) * c * c;
      m[3] = ((rx / ry) - (ry / rx)) * s * c;
      
      /* draw semi-ellipses */
      _plotter->fmove (p0.x, p0.y);
      _fakearc (p0, p1, HALF_ARC, m);
      _fakearc (p1, p0, HALF_ARC, m);
    }
      
  /* reset to original value */
  _plotter->drawstate->suppress_polyline_flushout = flushoutp;

  /* restore original value of drawing attribute */
  _plotter->joinmod (old_join_mode);
  free (old_join_mode);

  return;
}

/* The _fakearc() routine below contains our basic subdivision algorithm, a
   remote descendent of the arc-drawing algorithm of Ken Turkowski
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
   subdivision step.  One can attempt to remove one of them by noticing
   that the chord half-length h always satisfies h = sqrt(s * (2-s))).  So
   one can rotate the chord vector by 90 degrees, and multiply its length
   by s/2h, i.e., s/2sqrt(s * (2-s)), to get the chordal deviation vector.
   This factor still includes a square root though.  Also one still needs
   to compute a square root in order to proceed from one subdivision step
   to the next, i.e. to compute s' from s.

   We get around the square root problem by drawing only circular arcs with
   subtended angle of 90 degrees (quarter-circles), or elliptic arcs that
   are obtained from such quarter-circles by affine transformations
   (so-called quarter-ellipses).  To draw the latter, we need only replace
   the 90 degree rotation matrix mentioned above by an affine
   transformation that maps v0->-v1 and v1->v0, where v0 = p0 - pc and v1 =
   p1 - pc are the displacement vectors from the center of the ellipse to
   the endpoints of the arc.  If we do this, we get an elliptic arc with p0
   and p1 as endpoints. The vectors v0 and v1 are said lie along conjugate
   diameters of the quarter-ellipse.
   
   So for drawing quarter-ellipses, the only initial value of s we need to
   consider is the one for a quarter-circle, which is 1-sqrt(1/2).  The
   successive values of s/2h that will be encountered, after each
   bisection, are pre-computed and stored in a lookup table, found in
   fakearc.h.

   This approach lets us avoid, completely, any computation of square roots
   during the drawing of quarter-circles and quarter-ellipses.  The only
   square roots we need are precomputed.  We don't need any floating point
   divisions in the main loop either.  

   The implementation below does not use recursion (we use a local array,
   rather than the call stack, to store the sequence of generated
   points). */

#define SAME_POINT(p0, p1) (_plotter->drawstate->transform.is_raster ? \
			     ((IROUND(XD((p0).x, (p0).y)) \
			      == IROUND(XD((p1).x, (p1).y))) \
			     && (IROUND(YD((p0).x,(p0).y)) \
				 == IROUND(YD((p1).x,(p1).y)))) \
			    : ((p0).x == (p1).x && (p0).y == (p1).y))

static void 
#ifdef _HAVE_PROTOS
_fakearc(Point p0, Point p1, int arc_type, const double m[4])
#else
_fakearc(p0, p1, arc_type, m)
     Point p0, p1;
     int arc_type;
     const double m[4];
#endif
{
  Point p[MAX_ARC_SUBDIVISIONS + 1], q[MAX_ARC_SUBDIVISIONS + 1];
  int level[MAX_ARC_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < MAX_ARC_SUBDIVISIONS */
  int segments_drawn = 0;

  p[0] = p0;
  q[0] = p1;
  level[0] = 0;
  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      if (SAME_POINT(p[n], q[n]))
	n--;			/* discard zero-length line segment */
      
      else if (level[n] >= MAX_ARC_SUBDIVISIONS) 
	{			/* draw line segment */
	  _plotter->fcont (q[n].x, q[n].y);
	  segments_drawn++;
	  n--;
	}
      
      else			/* bisect line segment */
	{
	  Vector v;
	  Point pm, pb;
	  
	  v.x = q[n].x - p[n].x; /* chord = line segment from p[n] to q[n] */
	  v.y = q[n].y - p[n].y;
	  
	  pm.x = p[n].x + 0.5 * v.x; /* midpoint of chord */
	  pm.y = p[n].y + 0.5 * v.y;
	  
	  /* Compute bisection point.  If m=[0 1 -1 0] this just rotates
	     the chord clockwise by 90 degrees, and scales it to yield the
	     chordal deviation vector, which is used as an offset. */
	  
	  pb.x = pm.x + 
	    _chord_factor[arc_type][level[n]] * (m[0] * v.x + m[1] * v.y);
	  pb.y = pm.y + 
	    _chord_factor[arc_type][level[n]] * (m[2] * v.x + m[3] * v.y);
	  
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

  if (segments_drawn == 0)
    _plotter->fcont (q[0].x, q[0].y); /* draw at least one */

  return;
}

/* prepare_chord_table() tabulates the factors _fakearc() needs when it is
   employed to draw a circular arc of subtended angle other than the
   default angles it supports */
static void
#ifdef _HAVE_PROTOS
_prepare_chord_table (double sagitta)
#else
_prepare_chord_table (sagitta)
     double sagitta;
#endif
{
  double half_chord_length;
  int i;

  half_chord_length = sqrt ( sagitta * (2.0 - sagitta) );
  for (i = 0; i < TABULATED_ARC_SUBDIVISIONS; i++)
    {
      _chord_factor[USER_DEFINED_ARC][i] = 0.5 * sagitta / half_chord_length;
      sagitta = 1.0 - sqrt (1.0 - 0.5 * sagitta);
      half_chord_length = 0.5 * half_chord_length / (1.0 - sagitta);
    }

  return;
}
