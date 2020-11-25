/* This file includes vector-manipulation support routines used by some
   versions of libplot. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

/* scales an input vector to a new length and returns it */
Vector *
_vscale(v, newlen) 
     Vector *v;
     double newlen;
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
_xatan2 (y, x)
     double y, x;
{
  if (y == 0.0)
    return 0.0;
  else if (x == 0.0 && y >= 0.0)
    return M_PI_2;
  else if (x == 0.0 && y < 0.0)
    return -(M_PI_2);
  else
    return atan2(y, x);
}
