/* This file contains the internal function _draw_elliptic_arc(), which
   draws a counterclockwise elliptic arc using a elliptic arc subdivision
   algorithm.  The arc runs from point p0 to point p1, with center pc.  In
   the case when (p0-pc) x (p1-pc) is positive, we choose the elliptic arc
   so that it has control points p0, p1, and p0 + p1 - pc.  This means that
   it will be tangent at p0 to the line segment from p0 to p0 + p1 - pc,
   and tangent at p1 to the line segment from p1 to p0 + p1 - pc.  So it
   fits snugly into a triangle with vertices p0, p1, p0 + p1 - pc.  Notice
   that the control point p0 + p1 - pc is simply the reflection of pc
   through the line determined by p0 and p1.

   If p1 and p0 are collinear with pc, then nothing is drawn since the arc
   is not uniquely determined (and attempting to draw it will result in a
   division by zero).

   The polyline will contain no more than 2**MAX_ARC_SUBDIVISIONS line
   segments, since the subdividing stops when MAX_ARC_SUBDIVISIONS have
   been made.  MAX_ARC_SUBDIVISIONS, defined in extern.h, should be no
   greater than ARC_SUBDIVISIONS, defined in fakearc.h as the size of a
   lookup table.  On a raster device, we also stop subdividing when the
   line segments become zero pixels long.

   This sort of elliptic arc is called a `quarter-ellipse', since it is an
   affinely transformed quarter-circle.  Specifically, it is an affinely
   transformed version of the first quadrant of a unit circle, with
   the affine transformation mapping (0,0) to pc, (0,1) to p0, (1,0) to p1,
   and (1,1) to the control point p0 + p1 - pc.

   All the above applies when (p0-pc) x (p1-pc) is positive.  When it is
   negative, we draw a three-quarter ellipse, the definition of which is
   left to the reader.

   This file contains, as well, _draw_ellipse(), which draws two
   half-ellipses by a subdivision technique similar to that used by
   _draw_elliptic_arc, and _draw_circular_arc(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "fakearc.h"

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
void _draw_elliptic_arc_internal P__((Point p0, Point p1, Point pc));
#undef P__

#define DIST(p0,p1) (sqrt( ((p0).x - (p1).x)*((p0).x - (p1).x) \
			  + ((p0).y - (p1).y)*((p0).y - (p1).y)))

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
   consider is the one for a quarter-circle, which is 1-sqrt(1/2).
   [Actually we want to be able to draw three-quarter-ellipses too, so we
   also allow s = 1+sqrt(1/2) as an initial value.]  The successive values
   of s/2h that will be encountered, after each bisection, are pre-computed
   and stored in a lookup table, found in fakearc.h.

   This approach lets us avoid, completely, any computation of square roots
   during the drawing of quarter-circles and quarter-ellipses.  The only
   square roots we need are precomputed.  We don't need any floating point
   divisions in the main loop either.  

   The implementation below does not use recursion (we use a local array,
   rather than the call stack, to store the sequence of generated
   points). */

#define SAME_POINT(p0, p1) (_drawstate->transform.is_raster ? \
			     ((IROUND(XD((p0).x, (p0).y)) \
			      == IROUND(XD((p1).x, (p1).y))) \
			     && (IROUND(YD((p0).x,(p0).y)) \
				 == IROUND(YD((p1).x,(p1).y)))) \
			    : ((p0).x == (p1).x && (p0).y == (p1).y))

void 
_fakearc(p0, p1, arc_type, m)
     Point p0, p1;
     int arc_type;
     const double m[4];
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
	  fcont (q[n].x, q[n].y);
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
    fcont (q[0].x, q[0].y);	/* draw at least one */

  return;
}

/* prepare_chord_table() tabulates the factors _fakearc() needs when it is
   employed to draw a circular arc of subtended angle other than the
   default angles it supports */
void
_prepare_chord_table (sagitta)
     double sagitta;
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

/* _draw_elliptic_arc() approximates a particular counterclockwise elliptic
   arc from p0 to p1 by drawing an inscribed polyline, using _fakearc() and
   preserving drawing attributes.  pc is the center of the arc.

   If the cross product (p0 - pc) x (p1 - pc) is positive, the elliptic arc
   is chosen to be a quarter-ellipse (the vectors from pc to p0 and p1 will
   lie along the conjugate diameters).  In this case the control triangle
   for the elliptic arc will have vertices p0, p1, and K = p0 + (p1 - pc) =
   p1 + (p0 - pc).  The arc will pass through p0 and p1, and will be
   tangent at p0 to the edge from p0 to K, and at p1 to the edge from p1 to K.

   If the cross product (p0 - pc) x (p1 - pc) is negative, the elliptic arc
   will be a three-quarter ellipse from p1 to p0. */
void 
_draw_elliptic_arc (p0, p1, pc)
     Point p0, p1, pc; 
{ 
  Vector v0, v1; 
  double cross;
  double m[4];
  char *old_cap_mode, *old_join_mode;

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

  cross = v0.x * v1.y - v1.x * v0.y;

  if (cross == 0.0)
    {
      fprintf (stderr, "libplot: draw_elliptic_arc() called with collinear points, no arc drawn\n");
      return;
    }
  
  /* `rotation' matrix (it maps v0 -> -v1 and v1 -> v0) */
  m[0] = - (v0.x * v0.y + v1.x * v1.y) / cross;
  m[1] = (v0.x * v0.x + v1.x * v1.x) / cross;
  m[2] = - (v0.y * v0.y + v1.y * v1.y) / cross;
  m[3] = (v0.x * v0.y + v1.x * v1.y) / cross;

  /* save current values of relevant drawing attributes */
  old_cap_mode = (char *)_plot_xmalloc (strlen (_drawstate->cap_mode) + 1);
  old_join_mode = (char *)_plot_xmalloc (strlen (_drawstate->join_mode) + 1);
  
  strcpy (old_cap_mode, _drawstate->cap_mode);
  strcpy (old_join_mode, _drawstate->join_mode);
  
  capmod ("butt");	/* set these while drawing */
  joinmod ("round");
  
  /* draw inscribed polyline */
  if (cross > 0.0)
    {
      fmove (p0.x, p0.y);
      _fakearc (p0, p1, QUARTER_ARC, m);
    }
  else
    {
      fmove (p1.x, p1.y);      
      _fakearc (p1, p0, THREE_QUARTER_ARC, m);    
    }

  /* restore original values of drawing attributes */
  capmod (old_cap_mode);
  joinmod (old_join_mode);
  
  free (old_cap_mode);
  free (old_join_mode);
}

/* Draw inscribed polyline approximating an ellipse, with center (xc,yc)
   and semi-axes of length rx and ry (the former inclined at a specified
   angle to the x-axis).  This calls _fakearc() twice to draw polygonal
   approximations to two semi-ellipses.  If v0 = rx (cos theta, sin theta)
   and v1 = ry (-sin theta, cos theta) are the two semi-axes of the
   ellipse, the semi-ellipses go counterclockwise from p0 = pc + v0 to 
   p1 = pc + v1, and back.  Drawing attributes are saved and restored. */
void
_draw_ellipse (xc, yc, rx, ry, angle)
     double xc, yc, rx, ry, angle;
{
  Point p0, p1;

  double s = sin(M_PI * angle / 180.0);
  double c = cos(M_PI * angle / 180.0);  
  double m[4];

  /* save current values of relevant drawing attributes */
  char *old_cap_mode, *old_join_mode;
  
  if ((rx == 0.0) || (ry == 0.0))
    {
      fprintf (stderr, "libplot: _draw_ellipse() called with a zero semi-axis, no ellipse drawn");
      return;
    }

  old_cap_mode = (char *)_plot_xmalloc (strlen (_drawstate->cap_mode) + 1);
  old_join_mode = (char *)_plot_xmalloc (strlen (_drawstate->join_mode) + 1);
  
  strcpy (old_cap_mode, _drawstate->cap_mode);
  strcpy (old_join_mode, _drawstate->join_mode);
  
  capmod ("butt");		/* set these while drawing */
  joinmod ("round");
  
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
  fmove (p0.x, p0.y);
  _fakearc (p0, p1, HALF_ARC, m);
  _fakearc (p1, p0, HALF_ARC, m);

  /* restore original values of drawing attributes */
  capmod (old_cap_mode);
  joinmod (old_join_mode);
  
  free (old_cap_mode);
  free (old_join_mode);
}

/* _draw_circular_arc draws a counterclockwise arc, centered on pc, from p0
   to p1.  The radius is the distance from pc to p0; the position of p1 is
   adjusted so that its distance is the same. */
void
_draw_circular_arc(p0, p1, pc)
     Point p0, p1, pc; 
{
  /* bisection point of arc, and midpoint of chord */
  Point pb, pm;
  /* rotation matrix */
  double m[4];
  /* other variables */
  Vector v;
  double radius, sagitta;
  char *old_cap_mode, *old_join_mode;

  radius = DIST(pc, p0);	/* radius is distance to p0, not p1 */

  v.x = p1.x - pc.x;
  v.y = p1.y - pc.y;
  _vscale(&v, radius);
  p1.x = pc.x + v.x;		/* correct the location of p1 */
  p1.y = pc.y + v.y;

  v.x = p1.x - p0.x;		/* chord vector from p0 to p1 */
  v.y = p1.y - p0.y;
      
  _vscale(&v, radius);
  pb.x = pc.x + v.y;		/* bisection point of arc */
  pb.y = pc.y - v.x;
      
  pm.x = 0.5 * (p0.x + p1.x);	/* midpoint of chord */
  pm.y = 0.5 * (p0.y + p1.y);  

  sagitta = DIST(pb, pm) / radius;

  /* fill in entries of chord factor table for this user defined sagitta */
  _prepare_chord_table (sagitta);

  /* save current values of relevant drawing attributes */
  old_cap_mode = (char *)_plot_xmalloc (strlen (_drawstate->cap_mode) + 1);
  old_join_mode = (char *)_plot_xmalloc (strlen (_drawstate->join_mode) + 1);
  
  strcpy (old_cap_mode, _drawstate->cap_mode);
  strcpy (old_join_mode, _drawstate->join_mode);
  
  capmod ("butt");	/* set these while drawing */
  joinmod ("round");
  
  fmove (p0.x, p0.y);
  m[0] = 0.0, m[1] = 1.0, m[2] = -1.0, m[3] = 0.0;
  _fakearc (p0, p1, USER_DEFINED_ARC, m);

  /* restore original values of drawing attributes */
  capmod (old_cap_mode);
  joinmod (old_join_mode);
  
  free (old_cap_mode);
  free (old_join_mode);

  return;
}
