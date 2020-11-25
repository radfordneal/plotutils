#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* bit fields for return value from Cohen-Sutherland clipper */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

/* for internal clipper use */
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };

/* forward references */
static int _compute_outcode __P ((double x, double y, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip));

/* _clip_line() takes two points, the endpoints of a line segment in the
 * device frame (expressed in terms of floating-point device coordinates),
 * and destructively passes back two points: the endpoints of the line
 * segment clipped by Cohen-Sutherland to the rectangular clipping area.
 * The return value contains bitfields ACCEPTED, CLIPPED_FIRST, and
 * CLIPPED_SECOND.
 */

int
#ifdef _HAVE_PROTOS
_clip_line (double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip)
#else
_clip_line (x0_p, y0_p, x1_p, y1_p, x_min_clip, x_max_clip, y_min_clip, y_max_clip)
     double *x0_p, *y0_p, *x1_p, *y1_p;
     double x_min_clip, x_max_clip, y_min_clip, y_max_clip;
#endif
{
  double x0 = *x0_p;
  double y0 = *y0_p;
  double x1 = *x1_p;
  double y1 = *y1_p;
  int outcode0, outcode1;
  bool accepted;
  int clipval = 0;
  
  outcode0 = _compute_outcode (x0, y0, x_min_clip, x_max_clip, y_min_clip, y_max_clip);
  outcode1 = _compute_outcode (x1, y1, x_min_clip, x_max_clip, y_min_clip, y_max_clip);  

  do
    {
      if (!(outcode0 | outcode1)) /* accept */
	{
	  accepted = true;
	  break;
	}
      else if (outcode0 & outcode1) /* reject */
	{
	  accepted = false;
	  break;
	}
      else
	{
	  /* at least one endpoint is outside; choose one that is */
	  int outcode_out = (outcode0 ? outcode0 : outcode1);
	  double x, y;		/* intersection with clip edge */
	  
	  if (outcode_out & RIGHT)	  
	    {
	      x = x_max_clip;
	      y = y0 + (y1 - y0) * (x_max_clip - x0) / (x1 - x0);
	    }
	  else if (outcode_out & LEFT)
	    {
	      x = x_min_clip;
	      y = y0 + (y1 - y0) * (x_min_clip - x0) / (x1 - x0);
	    }
	  else if (outcode_out & TOP)
	    {
	      x = x0 + (x1 - x0) * (y_max_clip - y0) / (y1 - y0);
	      y = y_max_clip;
	    }
	  else
	    {
	      x = x0 + (x1 - x0) * (y_min_clip - y0) / (y1 - y0);
	      y = y_min_clip;
	    }
	  
	  if (outcode_out == outcode0)
	    {
	      x0 = x;
	      y0 = y;
	      outcode0 = _compute_outcode (x0, y0, x_min_clip, x_max_clip, y_min_clip, y_max_clip);
	    }
	  else
	    {
	      x1 = x; 
	      y1 = y;
	      outcode1 = _compute_outcode (x1, y1, x_min_clip, x_max_clip, y_min_clip, y_max_clip);
	    }
	}
    }
  while (true);

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

static int
#ifdef _HAVE_PROTOS
_compute_outcode (double x, double y, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip)
#else
_compute_outcode (x, y, x_min_clip, x_max_clip, y_min_clip, y_max_clip)
     double x, y, x_min_clip, x_max_clip, y_min_clip, y_max_clip;
#endif
{
  int code = 0;

  if (x > x_max_clip)
    code |= RIGHT;
  else if (x < x_min_clip)
    code |= LEFT;
  if (y > y_max_clip)
    code |= TOP;
  else if (y < y_min_clip)
    code |= BOTTOM;
  
  return code;
}
