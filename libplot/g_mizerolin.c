/* This file contains the miZeroLine() and miZeroDash() functions.  Each of
   them draws a single-pixel (i.e., `zero-width') Bresenham polyline, solid
   or dashed, respectively. */

/* Historical note: this is a sort of merger of MI code from X11, written
   by Ken Whaley, with low-level CFB (color frame-buffer) code, of unknown
   authorship.  The MI code included miZeroLine(), but not miZeroDash().
   I based the latter on the Bresenham CFB routines.  I needed to hack them
   extensively so that they would draw pixels using miFillSpans(), like
   miZeroLine(). */

#include "sys-defines.h"
#include "extern.h"

#include "g_mi.h"
#include "g_miline.h"

/* Comments: 

   NON-SLOPED LINES
   horizontal lines are always drawn left to right; we have to
move the endpoints right by one after they're swapped.
   vertical lines are always drawn top to bottom (y-increasing).
this requires adding one to the y-coordinate of each endpoint
after swapping.

   SLOPED LINES
   when clipping a sloped line, we bring the second point inside
the clipping box, rather than one beyond it, and then add 1 to
the length of the line before drawing it.  this lets us use
the same box for finding the outcodes for both endpoints.  since
the equation for clipping the second endpoint to an edge gives us
1 beyond the edge, we then have to move the point towards the
first point by one step on the major axis.
   eventually, there will be a diagram here to explain what's going
on.  the method uses Cohen-Sutherland outcodes to determine
outsideness, and a method similar to Pike's layers for doing the
actual clipping.  */

/* forward references */
static int miZeroClipLine ____P((int xmin, int ymin, int xmax, int ymax, int *new_x1, int *new_y1, int *new_x2, int *new_y2, unsigned int adx, unsigned int ady, int *pt1_clipped, int *pt2_clipped, int axis, bool signdx_eq_signdy, int oc1, int oc2));
static void cfbBresD ____P((miGC *pGC, int *pdashIndex, const unsigned int *pDash, int numInDashList, int *pdashOffset, bool isDoubleDash, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len, miIntPoint *pspanInit, unsigned int *pwidthInit));
static void cfbBresS ____P((miGC *pGC, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len, miIntPoint *pspanInit, unsigned int *pwidthInit));
static void cfbHorzS ____P((miGC *pGC, int x1, int y1, int len, miIntPoint *pspanInit, unsigned int *pwidthInit));
static void cfbVertS ____P((miGC *pGC, int x1, int y1, int len, miIntPoint *pspanInit, unsigned int *pwidthInit));


/* round, but maps x/y == z.5 to z.0 instead of (z+1).0 */
/* note that "ceiling" breaks for numerator < 1, so special-case it */
#define round_down(x, y)   ((int)(2*(x)-(y)) <= 0 ? 0 :\
			                (ceiling((2*(x)-(y)), (2*(y)))))

/* miZeroClipLine(): utility function that does Cohen-Sutherland clipping.
 *
 * returns:  1 for partially clipped line
 *          -1 for completely clipped line
 *
 */
static int
#ifdef _HAVE_PROTOS
miZeroClipLine (int xmin, int ymin, int xmax, int ymax, int *new_x1, int *new_y1, int *new_x2, int *new_y2, unsigned int adx, unsigned int ady, int *pt1_clipped, int *pt2_clipped, int axis, bool signdx_eq_signdy, int oc1, int oc2)
#else
miZeroClipLine (xmin, ymin, xmax, ymax, new_x1, new_y1, new_x2, new_y2, adx, ady, pt1_clipped, pt2_clipped, axis, signdx_eq_signdy, oc1, oc2)
     int xmin, ymin, xmax, ymax;
     int *new_x1, *new_y1, *new_x2, *new_y2;
     unsigned int adx, ady;
     int *pt1_clipped, *pt2_clipped;
     int axis;
     bool signdx_eq_signdy;
     int oc1, oc2;
#endif
{
  bool swapped = false;
  int clipDone = 0;
  unsigned int utmp;
  int clip1, clip2;
  int x1, y1, x2, y2;
  int x1_orig, y1_orig, x2_orig, y2_orig;

  x1 = x1_orig = *new_x1;
  y1 = y1_orig = *new_y1;
  x2 = x2_orig = *new_x2;
  y2 = y2_orig = *new_y2;
    
  clip1 = 0;
  clip2 = 0;

  do
    {
      if ((oc1 & oc2) != 0)
	{
	  clipDone = -1;
	  clip1 = oc1;
	  clip2 = oc2;
	}
      else if ((oc1 | oc2) == 0) /* trivial accept */
	{
	  clipDone = 1;
	  if (swapped)
	    {
	      SWAPINT_PAIR(x1, y1, x2, y2);
	      SWAPINT(oc1, oc2);
	      SWAPINT(clip1, clip2);
	    }
	}
      else			/* have to clip */
	{
	  /* only clip one point at a time */
	  if (oc1 == 0)
	    {
	      SWAPINT_PAIR(x1, y1, x2, y2);
	      SWAPINT_PAIR(x1_orig, y1_orig, x2_orig, y2_orig);
	      SWAPINT(oc1, oc2);
	      SWAPINT(clip1, clip2);
	      swapped = swapped ? false : true;
	    }
	    
	  clip1 |= oc1;
	  if (oc1 & OUT_LEFT)
	    {
	      if (axis == X_AXIS)
		{
		  utmp = xmin - x1_orig;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp *= ady;
		      if (signdx_eq_signdy)
			y1 = y1_orig + round(utmp, adx);
		      else
			y1 = y1_orig - round(utmp, adx);
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (x2_orig - xmin) * ady;
		      if (signdx_eq_signdy)
			y1 = y2_orig - round_down(utmp, adx);
		      else
			y1 = y2_orig + round_down(utmp, adx);
		    }
		}
	      else		/* Y_AXIS */
		{
		  utmp = xmin - x1_orig;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp = ((utmp * ady) << 1) - ady;
		      if (signdx_eq_signdy)
			y1 = y1_orig + ceiling(utmp, 2*adx);
		      else
			y1 = y1_orig - (utmp / (2*adx)) - 1;
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (((x2_orig - xmin) * ady) << 1) + ady;
		      if (signdx_eq_signdy)
			y1 = y2_orig - (utmp / (2*adx));
		      else
			y1 = y2_orig + ceiling(utmp, 2*adx) - 1;
		    }
		}
	      x1 = xmin;
	    }
	  else if (oc1 & OUT_ABOVE)
	    {
	      if (axis == Y_AXIS)
		{
		  utmp = ymin - y1_orig;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp *= adx;
		      if (signdx_eq_signdy)
			x1 = x1_orig + round(utmp, ady);
		      else
			x1 = x1_orig - round(utmp, ady);
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (y2_orig - ymin) * adx;
		      if (signdx_eq_signdy)
			x1 = x2_orig - round_down(utmp, ady);
		      else
			x1 = x2_orig + round_down(utmp, ady);
		    }
		}
	      else		/* X_AXIS */
		{
		  utmp = ymin - y1_orig;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp = ((utmp * adx) << 1) - adx;
		      if (signdx_eq_signdy)
			x1 = x1_orig + ceiling(utmp, 2*ady);
		      else
			x1 = x1_orig - (utmp / (2*ady)) - 1;
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (((y2_orig - ymin) * adx) << 1) + adx;
		      if (signdx_eq_signdy)
			x1 = x2_orig - (utmp / (2*ady));
		      else
			x1 = x2_orig + ceiling(utmp, 2*ady) - 1;
		    }
		}
	      y1 = ymin;
	    }
	  else if (oc1 & OUT_RIGHT)
	    {
	      if (axis == X_AXIS)
		{
		  utmp = x1_orig - xmax;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp *= ady;
		      if (signdx_eq_signdy)
			y1 = y1_orig - round_down(utmp, adx);
		      else
			y1 = y1_orig + round_down(utmp, adx);
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (xmax - x2_orig) * ady;
		      if (signdx_eq_signdy)
			y1 = y2_orig + round(utmp, adx);
		      else
			y1 = y2_orig - round(utmp, adx);
		    }
		}
	      else		/* Y_AXIS */
		{
		  utmp = x1_orig - xmax;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp = ((utmp * ady) << 1) - ady;
		      if (signdx_eq_signdy)
			y1 = y1_orig - (utmp / (2*adx)) - 1;
		      else
			y1 = y1_orig + ceiling(utmp, 2*adx);
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (((xmax - x2_orig) * ady) << 1) + ady;
		      if (signdx_eq_signdy)
			y1 = y2_orig + ceiling(utmp, 2*adx) - 1;
		      else
			y1 = y2_orig - (utmp / (2*adx));
		    }
		}
	      x1 = xmax;		
	    }
	  else if (oc1 & OUT_BELOW)
	    {
	      if (axis == Y_AXIS)
		{
		  utmp = y1_orig - ymax;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp *= adx;
		      if (signdx_eq_signdy)
			x1 = x1_orig - round_down(utmp, ady);
		      else
			x1 = x1_orig + round_down(utmp, ady);
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (ymax - y2_orig) * adx;
		      if (signdx_eq_signdy)
			x1 = x2_orig + round(utmp, ady);
		      else
			x1 = x2_orig - round(utmp, ady);
		    }
		}
	      else		/* X_AXIS */
		{
		  utmp = y1_orig - ymax;
		  if (utmp <= (unsigned int)INT_MAX)
		    {		/* clip using x1,y1 as a starting point */
		      utmp = ((utmp * adx) << 1) - adx;
		      if (signdx_eq_signdy)
			x1 = x1_orig - (utmp / (2*ady)) - 1;
		      else
			x1 = x1_orig + ceiling(utmp, 2*ady);
		    }
		  else		/* clip using x2,y2 as a starting point */
		    {
		      utmp = (((ymax - y2_orig) * adx) << 1) + adx;
		      if (signdx_eq_signdy)
			x1 = x2_orig + ceiling(utmp, 2*ady) - 1;
		      else
			x1 = x2_orig - (utmp / (2*ady));
		    }
		}
	      y1 = ymax;
	    }
	} /* else have to clip */
	
      oc1 = 0;
      oc2 = 0;
      MI_OUTCODES(oc1, x1, y1, xmin, ymin, xmax, ymax);
      MI_OUTCODES(oc2, x2, y2, xmin, ymin, xmax, ymax);
	
    } while (!clipDone);
    
  *new_x1 = x1;
  *new_y1 = y1;
  *new_x2 = x2;
  *new_y2 = y2;
    
  *pt1_clipped = clip1;
  *pt2_clipped = clip2;
    
  return clipDone;
}


/* Our macro for generating a list of spans, used when the successive
   points on a Bresenham line are generated.  Assumes the availability of
   working storage, accessed via pointers `spans' and `widths', and also
   variables Nspans (initted to 0), new_span (initted to true), and
   current_y.  The arrays should be at least as large as the longest
   generated Bresenham line segment. */
   
#define MI_OUTPUT_POINT(xx, yy)\
{\
    if (!new_span && yy == current_y)\
    {\
        if (xx < spans->x)\
	    spans->x = xx;\
	++*widths;\
    }\
    else\
    {\
        ++Nspans;\
	++spans;\
	++widths;\
	spans->x = xx;\
	spans->y = yy;\
	*widths = 1;\
	current_y = yy;\
        new_span = false;\
    }\
}

/*
 * Draw a solid 1-pixel polyline, i.e. a `zero-width' solid polyline.
 */
void
#ifdef _HAVE_PROTOS
miZeroLine (miGC *pGC, int mode, int npt, const miIntPoint *pPts)
#else
miZeroLine (pGC, mode, npt, pPts)
     miGC	*pGC;
     int	mode;		/* Origin or Previous */
     int	npt;		/* number of points */
     const miIntPoint *pPts;	/* point array */
#endif
{
  /* variables in span generation code, i.e. in MI_OUTPUT_POINT() */
  int Nspans;
  int current_y;
  miIntPoint *spans;
  unsigned int *widths;
  bool new_span;

  miIntPoint *pspanInit;	/* work array for span generation */
  unsigned int *pwidthInit;	/* work array for span generation */

  const miIntPoint *ppt;	/* pointer to point within array */

  unsigned int oc1;		/* outcode of point 1 */
  unsigned int oc2;		/* outcode of point 2 */

  int adx;			/* abs values of dx and dy */
  int ady;
  int signdx;			/* sign of dx and dy */
  int signdy;
  int e, e1, e2;		/* Bresenham error and increments */
  int len;			/* length of segment */
  int axis;			/* major axis */

  /* temporaries */
  int y1, y2;
  int x1, x2;
  int xstart, ystart;

  /* for clipping */
  int xleft, xright, ytop, ybottom;

  xleft   = 0;
  ytop    = 0;
  xright  = pGC->width - 1;
  ybottom = pGC->height - 1;

  /* since we're clipping to the drawable's boundaries & coordinate
   * space boundaries, we're guaranteed that the larger of width/height
   * is the longest span we'll need to output
   */
  {
    unsigned int width, height, list_len;

    width = xright - xleft + 1;
    height = ybottom - ytop + 1;
    list_len = (height >= width) ? height : width;
    pspanInit = (miIntPoint *)mi_xmalloc(list_len * sizeof(miIntPoint));
    pwidthInit = (unsigned int *)mi_xmalloc(list_len * sizeof(unsigned int));
  }

  /* loop through points, drawing a solid Bresenham segment for each line
     segment */
  ppt = pPts;
  xstart = ppt->x;
  ystart = ppt->y;
  x2 = xstart;
  y2 = ystart;
  while (--npt)
    {
      x1 = x2;
      y1 = y2;
      ++ppt;

      x2 = ppt->x;
      y2 = ppt->y;
      if (mode == miCoordModePrevious)
	/* convert from relative coordinates */
	{
	  x2 += x1;
	  y2 += y1;
	}

      if (x1 == x2)  /* vertical line */
	{
	  if (y1 > y2)
	    /* make line go top to bottom, keeping endpoint semantics */
	    {
	      int tmp;

	      tmp = y2;
	      y2 = y1 + 1;
	      y1 = tmp + 1;
	    }

	  if (y1 <= ybottom && y2 >= ytop + 1)
	    {
	      int y1t, y2t;

	      y1t = IMAX(y1, ytop);
	      y2t = IMIN(y2, ybottom + 1);
	      if (y1t != y2t)
		cfbVertS (pGC, x1, y1t, y2t - y1t,
			  pspanInit, pwidthInit);
	    }

	  y2 = ppt->y;
	}

      else if (y1 == y2)  /* horizontal line */
	{
	  if (x1 > x2)
	    /* force line from left to right, keeping endpoint semantics */
	    {
	      int tmp;

	      tmp = x2;
	      x2 = x1 + 1;
	      x1 = tmp + 1;
	    }
	  
	  if (x1 <= xright && x2 >= xleft + 1)
	    {
	      int x1t, x2t;

	      x1t = IMAX(x1, xleft);
	      x2t = IMIN(x2, xright + 1);
	      if (x1t != x2t)
		cfbHorzS (pGC, x1t, y1, x2t - x1t,
			  pspanInit, pwidthInit);
	    }

	  x2 = ppt->x;
	}

      else	/* sloped line */
	{
	  signdx = 1;
	  if ((adx = x2 - x1) < 0)
	    {
	      adx = -adx;
	      signdx = -1;
	    }
	  signdy = 1;
	  if ((ady = y2 - y1) < 0)
	    {
	      ady = -ady;
	      signdy = -1;
	    }

	  if (adx > ady)
	    {
	      axis = X_AXIS;
	      e1 = ady << 1;
	      e2 = e1 - (adx << 1);
	      e = e1 - adx;
	      FIXUP_X_MAJOR_ERROR(e, signdx, signdy);
 	    }
	  else
	    {
	      axis = Y_AXIS;
	      e1 = adx << 1;
	      e2 = e1 - (ady << 1);
	      e = e1 - ady;
	      FIXUP_Y_MAJOR_ERROR(e, signdx, signdy);
	    }

	  /* we have Bresenham parameters and two points.
	     all we have to do now is clip and draw. */
	  oc1 = 0;
	  oc2 = 0;
	  MI_OUTCODES(oc1, x1, y1, xleft, ytop, xright, ybottom);
	  MI_OUTCODES(oc2, x2, y2, xleft, ytop, xright, ybottom);
	  if ((oc1 | oc2) == 0)
	    /* no need to clip; just draw */
	    {
	      if (axis == X_AXIS)
		len = adx;
	      else
		len = ady;
	      cfbBresS (pGC,
			signdx, signdy, axis, x1, y1, 
			e, e1, e2, len,
			pspanInit, pwidthInit);
	    }
	  else if (oc1 & oc2)
	    {
	      /* line doesn't intersect drawable; nothing to draw */
	    }
	  else
	    /* clip and draw */
	    {
	      int new_x1 = x1, new_y1 = y1, new_x2 = x2, new_y2 = y2;
	      int clip1 = 0, clip2 = 0;
	      int clipdx, clipdy;
	      int err;
		    
	      if (miZeroClipLine(xleft, ytop, xright, ybottom,
				 &new_x1, &new_y1, &new_x2, &new_y2,
				 (unsigned int)adx, (unsigned int)ady,
				 &clip1, &clip2, axis,
				 signdx == signdy ? true : false,
				 oc1, oc2) != -1)
		{
		  if (axis == X_AXIS)
		    len = abs(new_x2 - new_x1);
		  else
		    len = abs(new_y2 - new_y1);

		  /* if we've clipped the endpoint, always draw the full
		   * length of the segment, because then the capstyle
		   * doesn't matter */
		  if (clip2)
		    len++;

		  if (len)
		    {
		      /* unwind Bresenham error term to first point */
		      if (clip1)
			{
			  clipdx = abs(new_x1 - x1);
			  clipdy = abs(new_y1 - y1);
			  if (axis == X_AXIS)
			    err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			  else
			    err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
			}
		      else
			err = e;

		      /* draw the segment */
		      cfbBresS(pGC,
			       signdx, signdy, axis, new_x1, new_y1,
			       err, e1, e2, len,
			       pspanInit, pwidthInit);
		    } /* if len > 0 */
		} /* if ZeroClipLine... */
	    } /* clip and draw */
	} /* sloped line */
    } /* while (--npt) */

  /* reset variables used in MI_OUTPUT_POINT() */
  Nspans = 0;
  current_y = 0;
  new_span = true;
  spans  = pspanInit - 1;
  widths = pwidthInit - 1;

  /* paint the last point if the end style isn't CapNotLast.  (Assume that
     a projecting, butt, or round cap that is one pixel wide is the same as
     the single pixel of the endpoint.) */
  if (pGC->capStyle != miCapNotLast
      &&
      (xstart != x2 || ystart != y2 || ppt == pPts + 1))
    {
      if (x2 >= xleft && y2 >= ytop && x2 <= xright && y2 <= ybottom)
	MI_OUTPUT_POINT(x2,y2);
    }

  if (Nspans > 0)
    miFillSpans (pGC, Nspans, pspanInit, pwidthInit, true);

  free(pwidthInit);
  free(pspanInit);
}


/*
 * Draw a dashed 1-pixel polyline, i.e. a `zero-width' dashed polyline.
 */
void
#ifdef _HAVE_PROTOS
miZeroDash (miGC *pGC, int mode, int npt, const miIntPoint *pPts)
#else
miZeroDash (pGC, mode, npt, pPts)
     miGC *pGC;
     int mode;			/* Origin or Previous */
     int npt;			/* number of points */
     const miIntPoint *pPts;	/* point array */
#endif
{
  /* variables in span generation code, i.e. in MI_OUTPUT_POINT() */
  int Nspans;
  int current_y;
  miIntPoint *spans;
  unsigned int *widths;
  bool new_span;

  miIntPoint *pspanInit;	/* work array for span generation */
  unsigned int *pwidthInit;	/* work array for span generation */

  const miIntPoint *ppt;	/* pointer to current point */

  unsigned int oc1;		/* outcode of point 1 */
  unsigned int oc2;		/* outcode of point 2 */

  int adx;			/* abs values of dx and dy */
  int ady;
  int signdx;			/* sign of dx and dy */
  int signdy;
  int e, e1, e2;		/* Bresenham error and increments */
  int len;			/* length of segment */
  int axis;			/* major axis */

  /* temporaries */
  int x1, x2, y1, y2;
  int xstart, ystart;

  const unsigned int    *pDash;
  int		  dashOffset;
  int		  numInDashList;
  int		  dashIndex;
  bool		  isDoubleDash;
  int		  unclippedlen;

  /* for clipping */
  int xleft, xright, ytop, ybottom;

  xleft   = 0;
  ytop    = 0;
  xright  = pGC->width - 1;
  ybottom = pGC->height - 1;

  /* since we're clipping to the drawable's boundaries & coordinate
   * space boundaries, we're guaranteed that the larger of width/height
   * is the longest span we'll need to output
   */
  {
    unsigned int width, height, list_len;

    width = xright - xleft + 1;
    height = ybottom - ytop + 1;
    list_len = (height >= width) ? height : width;
    pspanInit = (miIntPoint *)mi_xmalloc(list_len * sizeof(miIntPoint));
    pwidthInit = (unsigned int *)mi_xmalloc(list_len * sizeof(unsigned int));
  }

  /* compute initial dash values */
  pDash = pGC->dash;
  numInDashList = pGC->numInDashList;
  isDoubleDash = (pGC->lineStyle == miLineDoubleDash ? true : false);
  dashIndex = 0;
  dashOffset = 0;
  miStepDash (pGC->dashOffset, &dashIndex, 
	      pDash, numInDashList, &dashOffset);

  /* loop through points, drawing a dashed Bresenham segment for each line
     segment of nonzero length */
  ppt = pPts;
  xstart = ppt->x;
  ystart = ppt->y;
  x2 = xstart;
  y2 = ystart;
  while (--npt)
    {
      x1 = x2;
      y1 = y2;
      ++ppt;

      x2 = ppt->x;
      y2 = ppt->y;
      if (mode == miCoordModePrevious)
	/* convert from relative coordinates */
	{
	  x2 += x1;
	  y2 += y1;
	}

      AbsDeltaAndSign(x2, x1, adx, signdx);
      AbsDeltaAndSign(y2, y1, ady, signdy);

      if (adx > ady)
	{
	  axis = X_AXIS;
	  e1 = ady << 1;
	  e2 = e1 - (adx << 1);
	  e = e1 - adx;
	  unclippedlen = adx;
	  FIXUP_X_MAJOR_ERROR(e, signdx, signdy);
	}
      else
	{
	  axis = Y_AXIS;
	  e1 = adx << 1;
	  e2 = e1 - (ady << 1);
	  e = e1 - ady;
	  unclippedlen = ady;
	  FIXUP_Y_MAJOR_ERROR(e, signdx, signdy);
	}

      /* we have Bresenham parameters and two points.
	 all we have to do now is clip and draw. */

      oc1 = 0;
      oc2 = 0;
      MI_OUTCODES(oc1, x1, y1, xleft, ytop, xright, ybottom);
      MI_OUTCODES(oc2, x2, y2, xleft, ytop, xright, ybottom);
      if ((oc1 | oc2) == 0)
	/* no need to clip; just draw */
	{
	  cfbBresD (pGC,
		    &dashIndex, pDash, numInDashList,
		    &dashOffset, isDoubleDash,
		    signdx, signdy, axis, x1, y1,
		    e, e1, e2, unclippedlen,
		    pspanInit, pwidthInit);
	  goto dontStep;
	}
      else if (oc1 & oc2)
	{
	  /* line doesn't intersect drawable; nothing to draw */
	}
      else
	/* clip and draw */
	{
	  int new_x1 = x1, new_y1 = y1, new_x2 = x2, new_y2 = y2;
	  int clip1 = 0, clip2 = 0;
	  int clipdx, clipdy;
	  int err;
	  int dashIndexTmp, dashOffsetTmp;
		
	  if (miZeroClipLine(xleft, ytop, xright, ybottom,
			     &new_x1, &new_y1, &new_x2, &new_y2,
			     (unsigned int)adx, (unsigned int)ady,
			     &clip1, &clip2, axis,
			     signdx == signdy ? true : false,
			     oc1, oc2) != -1)
	    {

	      dashIndexTmp = dashIndex;
	      dashOffsetTmp = dashOffset;

	      if (clip1)
		{
		  int dlen;
    
		  if (axis == X_AXIS)
		    dlen = abs(new_x1 - x1);
		  else
		    dlen = abs(new_y1 - y1);
		  miStepDash (dlen, &dashIndexTmp, pDash,
			      numInDashList, &dashOffsetTmp);
		}
		
	      if (axis == X_AXIS)
		len = abs(new_x2 - new_x1);
	      else
		len = abs(new_y2 - new_y1);

	      /* if we've clipped the endpoint, always draw the full length
	         of the segment, because then the capstyle doesn't matter */
	      if (clip2)
		len++;

	      if (len)
		{
		  /* unwind Bresenham error term to first point */
		  if (clip1)
		    {
		      clipdx = abs(new_x1 - x1);
		      clipdy = abs(new_y1 - y1);
		      if (axis == X_AXIS)
			err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
		      else
			err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
		    }
		  else
		    err = e;

		  /* draw the segment */
		  cfbBresD (pGC,
			    &dashIndexTmp, pDash, numInDashList,
			    &dashOffsetTmp, isDoubleDash,
			    signdx, signdy, axis, new_x1, new_y1,
			    err, e1, e2, len,
			    pspanInit, pwidthInit);
		} /* if len > 0 */
	    } /* if miZeroClipLine... */
	} /* clip and draw */

      /*
       * walk the dash list around to the next line
       */
      miStepDash (unclippedlen, &dashIndex, pDash,
		  numInDashList, &dashOffset);
    dontStep:	;
    } /* while (nline--) */

  /* reset variables used in MI_OUTPUT_POINT() */
  Nspans = 0;
  current_y = 0;
  new_span = true;
  spans  = pspanInit - 1;
  widths = pwidthInit - 1;

  /* paint the last point if the end style isn't CapNotLast.
     (Assume that a projecting, butt, or round cap that is one
     pixel wide is the same as the single pixel of the endpoint.) */
  if (pGC->capStyle != miCapNotLast
      &&
      (xstart != x2 || ystart != y2 || ppt == pPts + 1))
    {
      if (x2 >= xleft && y2 >= ytop && x2 <= xright && y2 <= ybottom)
	{
	  if (dashIndex & 1)
	    {
	      if (isDoubleDash)
		{
		  miPixel oldPixel;
		  
		  oldPixel = pGC->fgPixel;
		  pGC->fgPixel = pGC->bgPixel;
		  MI_OUTPUT_POINT(x2,y2);
		  pGC->fgPixel = oldPixel;
		}
	    }
	  else
	    MI_OUTPUT_POINT(x2,y2);
	}
    }

  if (Nspans > 0)
    miFillSpans (pGC, Nspans, pspanInit, pwidthInit, true);

  free(pwidthInit);
  free(pspanInit);
}


/* Internal: draw solid Bresenham line segment, no clipping (it's done
   at a higher level). */
static void
#ifdef _HAVE_PROTOS
cfbBresS (miGC *pGC, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len, miIntPoint *pspanInit, unsigned int *pwidthInit)
#else
cfbBresS (pGC, signdx, signdy, axis, x1, y1, e, e1, e2, len, pspanInit, pwidthInit)
     miGC	    *pGC;
     int    	    signdx;
     int	    signdy;		/* signs of directions */
     int	    axis;		/* major axis (Y_AXIS or X_AXIS) */
     int	    x1, y1;		/* initial point */
     int    	    e;			/* error accumulator */
     int    	    e1;			/* Bresenham increments */
     int	    e2;
     int	    len;		/* length of line */
     miIntPoint     *pspanInit;		/* work array for span generation */
     unsigned int   *pwidthInit;	/* work array for span generation */
#endif
{
  /* variables in span generation code, i.e. in MI_OUTPUT_POINT() */
  int Nspans;
  int current_y;
  miIntPoint *spans;
  unsigned int *widths;
  bool new_span;

  int x, y;
  int e3 = e2 - e1;

  if (len == 0)
    return;

  e = e - e1;			/* make looping easier */

  /* point to first point */
  x = x1;
  y = y1;

  /* reset variables used in MI_OUTPUT_POINT() */
  Nspans = 0;
  current_y = 0;
  new_span = true;
  spans  = pspanInit - 1;
  widths = pwidthInit - 1;

#define BresStep(minor,major) \
	{if ((e += e1) >= 0) { e += e3; minor; } major;}

#define Loop_x(counter,store) while (counter--) \
	{store; BresStep(y+=signdy,x+=signdx) }
#define Loop_y(counter,store) while (counter--) \
	{store; BresStep(x+=signdx,y+=signdy) }

  switch (axis)
    {
    case X_AXIS:
    default:
        Loop_x(len, MI_OUTPUT_POINT(x,y))
	break;
    case Y_AXIS:
        Loop_y(len, MI_OUTPUT_POINT(x,y))
	break;
    }

  if (Nspans > 0)
    miFillSpans (pGC, Nspans, pspanInit, pwidthInit, 
		 signdy >= 0 ? true : false);
}

/* Internal: draw dashed Bresenham line segment, no clipping (it's done
   at a higher level). */
static void
#ifdef _HAVE_PROTOS
cfbBresD (miGC *pGC, int *pdashIndex, const unsigned int *pDash, int numInDashList, int *pdashOffset, bool isDoubleDash, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len, miIntPoint *pspanInit, unsigned int *pwidthInit)
#else
cfbBresD (pGC, pdashIndex, pDash, numInDashList, pdashOffset, isDoubleDash,
	  signdx, signdy, axis, x1, y1, e, e1, e2, len, pspanInit, pwidthInit)
     miGC	    *pGC;
     int	    *pdashIndex;	/* current dash */
     const unsigned int   *pDash;	/* dash list */
     int	    numInDashList;	/* total length of dash list */
     int	    *pdashOffset;	/* offset into current dash */
     bool	    isDoubleDash;
     int            signdx, signdy;	/* signs of directions */
     int	    axis;		/* major axis (Y_AXIS or X_AXIS) */
     int	    x1, y1;		/* initial point */
     int	    e;			/* error accumulator */
     int    	    e1;			/* Bresenham increments */
     int	    e2;
     int	    len;		/* length of line */
     miIntPoint     *pspanInit;		/* work array for span generation */
     unsigned int   *pwidthInit;	/* work array for span generation */
#endif
{
  /* variables in span generation code, i.e. in MI_OUTPUT_POINT() */
  int Nspans;
  int current_y;
  miIntPoint *spans;
  unsigned int *widths;
  bool new_span;

  int		x, y;
  int 		e3 = e2-e1;
  int		dashIndex;
  int		dashOffset;
  int		dashRemaining;
  int		thisDash;

  dashIndex = *pdashIndex;	/* index of current dash */
  dashOffset = *pdashOffset;	/* offset into current dash */
  dashRemaining = (int)(pDash[dashIndex]) - dashOffset;	/* how much is left */
  if (len <= (thisDash = dashRemaining))
    /* line segment will be solid, not dashed */
    {
      thisDash = len;
      dashRemaining -= len;
    }
  e = e - e1;			/* make looping easier */

#define NextDash {\
    dashIndex++; \
    if (dashIndex == numInDashList) \
	dashIndex = 0; \
    dashRemaining = (int)(pDash[dashIndex]); \
    if ((thisDash = dashRemaining) >= len) \
    { \
	dashRemaining -= len; \
	thisDash = len; \
    } \
}

  /* point to first point */
  x = x1;
  y = y1;

  /* loop, generating dashes */
  for ( ; ; )
    { 
      len -= thisDash;

      /* reset variables used in MI_OUTPUT_POINT() */
      Nspans = 0;
      current_y = 0;
      new_span = true;
      spans  = pspanInit - 1;
      widths = pwidthInit - 1;

      switch (axis)
	{
	case X_AXIS:
	default:
	  if (dashIndex & 1) 
	    {
	      if (isDoubleDash) 
		{
		  miPixel oldPixel;
		  
		  oldPixel = pGC->fgPixel;
		  pGC->fgPixel = pGC->bgPixel;
		  Loop_x(thisDash, MI_OUTPUT_POINT(x,y))
		  pGC->fgPixel = oldPixel;
		}
	      else 
	        Loop_x(thisDash, ;);
	    } 
	  else
	    Loop_x(thisDash, MI_OUTPUT_POINT(x,y))
	  break;
	case Y_AXIS:
	  if (dashIndex & 1) 
	    {
	      if (isDoubleDash) 
		{
		  miPixel oldPixel;
		  
		  oldPixel = pGC->fgPixel;
		  pGC->fgPixel = pGC->bgPixel;
		  Loop_y(thisDash, MI_OUTPUT_POINT(x,y))
		  pGC->fgPixel = oldPixel;
		}
	      else 
	        Loop_y(thisDash, ;);
	    } 
	  else
	    Loop_y(thisDash, MI_OUTPUT_POINT(x,y))
	  break;
	}
      
      if (Nspans > 0)
	miFillSpans (pGC, Nspans, pspanInit, pwidthInit, 
		 signdy >= 0 ? true : false);

      if (len == 0)
	break;
      NextDash
    }

  *pdashIndex = dashIndex;
  *pdashOffset = (int)(pDash[dashIndex]) - dashRemaining;
}

/* Internal: draw horizontal zero-width solid line segment.
   No clipping (it's done at a higher level).
   Called with len >= 1, and len=x2-x1.  Endpoint semantics
   are used, so we paint only len pixels, i.e. x1..x2-1. */
static void
#ifdef _HAVE_PROTOS
cfbHorzS (miGC *pGC, int x1, int y1, int len, miIntPoint *pspanInit, unsigned int *pwidthInit)
#else
cfbHorzS (pGC, x1, y1, len, pspanInit, pwidthInit)
     miGC	*pGC;
     int	x1, y1;		/* initial point */ 
     int	len;		/* length of line */
     miIntPoint     *pspanInit; /* work array for span generation */
     unsigned int   *pwidthInit; /* work array for span generation */
#endif
{
  /* variables in span generation code, i.e. in MI_OUTPUT_POINT() */
  int Nspans;
  int current_y;
  miIntPoint *spans;
  unsigned int *widths;
  bool new_span;

  int i;

  /* reset variables used in MI_OUTPUT_POINT() */
  Nspans = 0;
  current_y = 0;
  new_span = true;
  spans  = pspanInit - 1;
  widths = pwidthInit - 1;

  for (i = 0; i < len; i++)
    MI_OUTPUT_POINT(x1 + i, y1);

  if (Nspans > 0)
    miFillSpans (pGC, Nspans, pspanInit, pwidthInit, true);
}


/* Internal: draw vertical zero-width solid line segment.
   No clipping (it's done at a higher level).
   Called with len >= 1, and len=y2-y1.  Endpoint semantics
   are used, so we paint only len pixels, i.e. y1..y2-1. */
static void
#ifdef _HAVE_PROTOS
cfbVertS (miGC *pGC, int x1, int y1, int len, miIntPoint *pspanInit, unsigned int *pwidthInit)
#else
cfbVertS (pGC, x1, y1, len, pspanInit, pwidthInit)
     miGC	*pGC;
     int	x1, y1;		/* initial point */
     int	len;		/* length of line */
     miIntPoint     *pspanInit; /* work array for span generation */
     unsigned int   *pwidthInit; /* work array for span generation */
#endif
{
  /* variables in span generation code, i.e. in MI_OUTPUT_POINT() */
  int Nspans;
  int current_y;
  miIntPoint *spans;
  unsigned int *widths;
  bool new_span;

  int i;

  /* reset variables used in MI_OUTPUT_POINT() */
  Nspans = 0;
  current_y = 0;
  new_span = true;
  spans  = pspanInit - 1;
  widths = pwidthInit - 1;

  for (i = 0; i < len; i++)
    MI_OUTPUT_POINT(x1, y1 + i);

  if (Nspans > 0)
    miFillSpans (pGC, Nspans, pspanInit, pwidthInit, true);
}
