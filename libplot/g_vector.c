/* This file includes vector-related utility routines. */

#include "sys-defines.h"
#include "extern.h"

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

/* Scale an input vector to a new length, and return it. */
plVector *
#ifdef _HAVE_PROTOS
_vscale(plVector *v, double newlen) 
#else
_vscale(v, newlen) 
     plVector *v;
     double newlen;
#endif
{
  double len = VLENGTH(*v);
  
  if (len != 0.0) 
    { 
      v->x *= newlen/len;   
      v->y *= newlen/len; 
    }
  return(v);
}

/* Compute angle (arctangent) of 2-D vector via atan2, but standardize
   handling of singular cases.  */
double
#ifdef _HAVE_PROTOS
_xatan2 (double y, double x)
#else
_xatan2 (y, x)
     double y, x;
#endif
{
  if (y == 0.0 && x >= 0.0)
    return 0.0;
  else if (y == 0.0 && x < 0.0)
    return M_PI;
  else if (x == 0.0 && y >= 0.0)
    return M_PI_2;
  else if (x == 0.0 && y < 0.0)
    return -(M_PI_2);
  else
    return atan2(y, x);
}

/* Compute angle between vectors pc..p0 and pc..pp1, in range -pi..pi;
   collinear vectors yield an angle of pi.  This is used when drawing arcs.  */
double
#ifdef _HAVE_PROTOS
_angle_of_arc(plPoint p0, plPoint pp1, plPoint pc)
#else
_angle_of_arc(p0, pp1, pc)
     plPoint p0, pp1, pc; 
#endif
{
  plVector v0, v1;
  double cross, angle, angle0;

  /* vectors from pc to p0, and pc to pp1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = pp1.x - pc.x;
  v1.y = pp1.y - pc.y;

  /* relative polar angle of p0 */
  angle0 = _xatan2 (v0.y, v0.x);

  /* cross product, zero means points are collinear */
  cross = v0.x * v1.y - v1.x * v0.y;

  if (cross == 0.0)
    /* by libplot convention, sweep angle should be M_PI not -(M_PI), in
       the collinear case */
    angle = M_PI;
  else
    /* compute angle in range -(M_PI)..M_PI */
    {
      double angle1;

      angle1 = _xatan2 (v1.y, v1.x);
      angle = angle1 - angle0;
      if (angle > M_PI)
	angle -= (2.0 * M_PI);
      else if (angle < -(M_PI))
	angle += (2.0 * M_PI);
    }
  
  return angle;
}

/* Adjust the location of pc so it can be used as the center of a circle or
   circular arc through p0 and p1.  If pc does not lie on the line that
   perpendicularly bisects the line segment from p0 to p1, it is projected
   orthogonally onto it.  p0 and p1 are assumed not to be coincident. */
plPoint
#ifdef _HAVE_PROTOS
_truecenter(plPoint p0, plPoint p1, plPoint pc)
#else
_truecenter(p0, p1, pc)
     plPoint p0, p1, pc;
#endif
{
  plPoint pm;
  plVector a, b, c;
  double scale;
  
  /* midpoint */
  pm.x = 0.5 * (p0.x + p1.x);
  pm.y = 0.5 * (p0.y + p1.y);  

  /* a points along perpendicular bisector */
  a.x = -p1.y + p0.y; 
  a.y =  p1.x - p0.x;

  /* b points from midpoint to pc */
  b.x = pc.x - pm.x;
  b.y = pc.y - pm.y;

  /* c is orthogonal projection of b onto a */
  scale = (a.x * b.x + a.y * b.y) / (a.x * a.x + a.y * a.y);
  c.x = scale * a.x;
  c.y = scale * a.y;

  /* adjust pc */
  pc.x = pm.x + c.x;
  pc.y = pm.y + c.y;

  return pc;
}  
