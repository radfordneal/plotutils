#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

typedef unsigned int outcode;	/* for Cohen-Sutherland clipper */
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static outcode _compute_outcode P__ ((double x, double y));
#undef P__

/* clip_line() takes two points, the endpoints of a line segment in the
 * device frame (expressed in terms of floating-point device coordinates),
 * and destructively passes back two points: the endpoints of the line
 * segment clipped by Cohen-Sutherland to the rectangular clipping area.
 * The return value contains bitfields ACCEPTED, CLIPPED_FIRST, and
 * CLIPPED_SECOND.
 */

int
clip_line (x0_p, y0_p, x1_p, y1_p)
     double *x0_p, *y0_p, *x1_p, *y1_p;
{
  double x0 = *x0_p;
  double y0 = *y0_p;
  double x1 = *x1_p;
  double y1 = *y1_p;
  outcode outcode0 = _compute_outcode (x0, y0);
  outcode outcode1 = _compute_outcode (x1, y1);  
  Boolean accepted;
  int clipval = 0;
  
  do
    {
      if (!(outcode0 | outcode1)) /* accept */
	{
	  accepted = TRUE;
	  break;
	}
      else if (outcode0 & outcode1) /* reject */
	{
	  accepted = FALSE;
	  break;
	}
      else
	{
	  /* at least one endpoint is outside; choose one that is */
	  outcode outcode_out = (outcode0 ? outcode0 : outcode1);
	  double x, y;		/* intersection with clip edge */
	  
	  if (outcode_out & RIGHT)	  
	    {
	      x = DEVICE_X_MAX_CLIP;
	      y = y0 + (y1 - y0) * (DEVICE_X_MAX_CLIP - x0) / (x1 - x0);
	    }
	  else if (outcode_out & LEFT)
	    {
	      x = DEVICE_X_MIN_CLIP;
	      y = y0 + (y1 - y0) * (DEVICE_X_MIN_CLIP - x0) / (x1 - x0);
	    }
	  else if (outcode_out & TOP)
	    {
	      x = x0 + (x1 - x0) * (DEVICE_Y_MAX_CLIP - y0) / (y1 - y0);
	      y = DEVICE_Y_MAX_CLIP;
	    }
	  else
	    {
	      x = x0 + (x1 - x0) * (DEVICE_Y_MIN_CLIP - y0) / (y1 - y0);
	      y = DEVICE_Y_MIN_CLIP;
	    }
	  
	  if (outcode_out == outcode0)
	    {
	      x0 = x;
	      y0 = y;
	      outcode0 = _compute_outcode (x0, y0);
	    }
	  else
	    {
	      x1 = x; 
	      y1 = y;
	      outcode1 = _compute_outcode (x1, y1);
	    }
	}
    }
  while (TRUE);

  if (accepted)
    {
      clipval |= ACCEPTED;
      if ((x0 != *x0_p) || (y0 != *y0_p))
	clipval |= CLIPPED_FIRST;
      if ((x1 != *x1_p) || (y1 != *y1_p))
	clipval |= CLIPPED_SECOND;
      *x0_p = x0;
      *y0_p = y0;
      *x1_p = x1;
      *y1_p = y1;
    }

  return clipval;
}

static outcode
_compute_outcode (x, y)
     double x, y;
{
  outcode code = 0;
  if (x > DEVICE_X_MAX_CLIP)
    code |= RIGHT;
  else if (x < DEVICE_X_MIN_CLIP)
    code |= LEFT;
  if (y > DEVICE_Y_MAX_CLIP)
    code |= TOP;
  else if (y < DEVICE_Y_MIN_CLIP)
    code |= BOTTOM;
  
  return code;
}
