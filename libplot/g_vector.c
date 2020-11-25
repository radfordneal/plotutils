/* This file includes vector-related utility routines. */

#include "sys-defines.h"
#include "extern.h"

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

/* scales an input vector to a new length and returns it */
Vector *
#ifdef _HAVE_PROTOS
_vscale(Vector *v, double newlen) 
#else
_vscale(v, newlen) 
     Vector *v;
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

/* compute angle (arctangent) of 2-D vector via atan2, but standardize
   handling of singular cases */
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

/* _fixcenter adjusts the location of the point pc so that it can be used
   as the center of a circle or circular arc through p0 and p1.  If pc does
   not lie on the line that perpendicularly bisects the line segment from
   p0 to p1, it is projected orthogonally onto it.  p0 and p1 are assumed
   not to be coincident. */
Point
#ifdef _HAVE_PROTOS
_truecenter(Point p0, Point p1, Point pc)
#else
_truecenter(p0, p1, pc)
     Point p0, p1, pc;
#endif
{
  Point pm;
  Vector a, b, c;
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
