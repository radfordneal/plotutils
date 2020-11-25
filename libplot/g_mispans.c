#include "sys-defines.h"
#include "extern.h"

#include "g_mi.h"
#include "g_mispans.h"

/* These routines maintain lists of Spans, in order to implement the
   ``touch-each-pixel-once'' rules of wide lines and arcs.
   
   Written by Joel McCormack, Summer 1989.
*/

/* internal functions */
static int UniquifySpansX ____P((const Spans *spans, miIntPoint *newPoints, unsigned int *newWidths));
static void QuickSortSpansX ____P((miIntPoint *points, unsigned int *widths, int numSpans));
static void miSubtractSpans ____P((SpanGroup *spanGroup, const Spans *sub));


/* Initialize SpanGroup, i.e. a set of Spans's, implemented as a partially
   filled array. */
void 
#ifdef _HAVE_PROTOS
miInitSpanGroup (SpanGroup *spanGroup)
#else
miInitSpanGroup (spanGroup)
     SpanGroup *spanGroup;
#endif
{
  spanGroup->size = 0;
  spanGroup->count = 0;
  spanGroup->group = NULL;
  spanGroup->ymin = INT_MAX;
  spanGroup->ymax = INT_MIN;
}

/* spans in each Spans are sorted by y, so these give ymin, ymax for a Spans */
#define YMIN(spans) (spans->points[0].y)
#define YMAX(spans)  (spans->points[spans->count-1].y)

/* Subtract a Spans from a SpanGroup. */
static void 
#ifdef _HAVE_PROTOS
miSubtractSpans (SpanGroup *spanGroup, const Spans *sub)
#else
miSubtractSpans (spanGroup, sub)
     SpanGroup *spanGroup;
     const Spans *sub;
#endif
{
  int		i, subCount, spansCount;
  int		ymin, ymax, xmin, xmax;
  Spans		*spans;
  miIntPoint	*subPt, *spansPt;
  unsigned int	*subWid, *spansWid;
  int		extra;

  /* y range of Spans to be subtracted */
  ymin = YMIN(sub);
  ymax = YMAX(sub);

  /* loop through all Spans's in SpanGroup */
  spans = spanGroup->group;
  for (i = spanGroup->count; i; i--, spans++) 
    {
      /* look only at Spans's with y ranges that overlap with `sub' */
      if (YMIN(spans) <= ymax && ymin <= YMAX(spans)) 
	{
	  /* count, start points, and widths for `sub' */
	  subCount = sub->count;
	  subPt = sub->points;
	  subWid = sub->widths;

	  /* count, start points, and widths for current Spans */
	  spansCount = spans->count;
	  spansPt = spans->points;
	  spansWid = spans->widths;

	  extra = 0;
	  for (;;)
	    /* look at pairs of spans, one from each Spans, that have the
	       same value for y (break out when one or the other Spans is
	       exhausted) */
	    {
	      while (spansCount && spansPt->y < subPt->y)
		{
		  spansPt++;  
		  spansWid++; 
		  spansCount--;
		}
	      if (!spansCount)
		break;
	      while (subCount && subPt->y < spansPt->y)
		{
		  subPt++;
		  subWid++;
		  subCount--;
		}
	      if (!subCount)
		break;
	      if (subPt->y == spansPt->y)
		/* same value of y, analyse in detail */
		{
		  xmin = subPt->x;
		  xmax = xmin + *subWid;
		  if (xmin >= spansPt->x + (int)*spansWid 
		      || spansPt->x >= xmax)
		    {
		      ;
		    }
		  else if (xmin <= spansPt->x)
		    {
		      if (xmax >= spansPt->x + (int)*spansWid)
			{
			  memmove (spansPt, spansPt + 1, 
				   sizeof *spansPt * (spansCount - 1));
			  memmove (spansWid, spansWid + 1, 
				   sizeof *spansWid * (spansCount - 1));
			  spansPt--;
			  spansWid--;
			  spans->count--;
			  extra++;
			}
		      else 
			{
			  *spansWid = *spansWid - (xmax - spansPt->x);
			  spansPt->x = xmax;
			}
		    }
		  else
		    {
		      if (xmax >= spansPt->x + (int)*spansWid)
			*spansWid = (unsigned int)(xmin - spansPt->x);
		      else
			{
			  if (!extra) 
			    {
			      miIntPoint *newPt;
			      unsigned int *newwid;

#define EXTRA 8
			      newPt = (miIntPoint *)mi_xrealloc (spans->points, 
							(spans->count + EXTRA) * sizeof (miIntPoint));
			      spansPt = newPt + (spansPt - spans->points);
			      spans->points = newPt;
			      newwid = (unsigned int *)mi_xrealloc (spans->widths, 
						       (spans->count + EXTRA) * sizeof (unsigned int));
			      spansWid = newwid + (spansWid - spans->widths);
			      spans->widths = newwid;
			      extra = EXTRA;
			    }
			  memmove (spansPt + 1, spansPt, 
				   sizeof *spansPt * (spansCount));
			  memmove (spansWid + 1, spansWid, 
				   sizeof *spansWid * (spansCount));
			  spans->count++;
			  extra--;
			  *spansWid = xmin - spansPt->x;
			  spansWid++;
			  spansPt++;
			  *spansWid = *spansWid - (xmax - spansPt->x);
			  spansPt->x = xmax;
			}
		    }
		}
	      /* on to next span in the Spans */
	      spansPt++;  
	      spansWid++; 
	      spansCount--;
	    }
	}
    }
}

/* Add a Spans to a SpanGroup, and possibly subtract it (remove it) from
   another.  The Spans's must be in y-sorted order. */
extern void
#ifdef _HAVE_PROTOS
miAppendSpans (SpanGroup *spanGroup, SpanGroup *otherGroup, const Spans *spans)
#else
miAppendSpans (spanGroup, otherGroup, spans)
     SpanGroup *spanGroup, *otherGroup;
     const Spans *spans;
#endif
{
  int ymin, ymax;
  int spansCount;

  spansCount = spans->count;
  if (spansCount > 0) 
    {
      if (spanGroup->size == spanGroup->count) 
	/* need to expand SpanGroup */
	{
	  spanGroup->size = (spanGroup->size + 8) * 2;
	  spanGroup->group = (Spans *)
	    mi_xrealloc(spanGroup->group, sizeof(Spans) * spanGroup->size);
	}

      /* tack Spans onto end of SpanGroup, update ymin and ymax for group */
      spanGroup->group[spanGroup->count] = *spans;
      (spanGroup->count)++;
      ymin = spans->points[0].y;
      if (ymin < spanGroup->ymin) 
	spanGroup->ymin = ymin;
      ymax = spans->points[spansCount - 1].y;
      if (ymax > spanGroup->ymax) 
	spanGroup->ymax = ymax;

      /* subtract Spans from `other' group if necessary */
      if (otherGroup && otherGroup->ymin < ymax && ymin < otherGroup->ymax)
	miSubtractSpans (otherGroup, spans);
    }
  else
    /* Spans doesn't contain even a single span, so deallocate its two
       arrays (if possible?). */
    {
      free (spans->points);
      free (spans->widths);
    }
}

/* Sort each span in a Spans by x.  Called only if numSpans > 1. */
static void 
#ifdef _HAVE_PROTOS
QuickSortSpansX (miIntPoint *points, unsigned int *widths, int numSpans)
#else
QuickSortSpansX (points, widths, numSpans)
     miIntPoint *points;
     unsigned int *widths;
     int numSpans;
#endif
{
  int	 x;
  int	 i, j, m;
  miIntPoint *r;

#define ExchangeSpans(a, b)				    \
  {							    \
    miIntPoint tpt;	     					    \
    unsigned int tw;					    \
								\
    tpt = points[a]; points[a] = points[b]; points[b] = tpt;    \
    tw = widths[a]; widths[a] = widths[b]; widths[b] = tw;  \
  }

  do 
    {
      if (numSpans < 9) 
	/* do insertion sort */
	{
	  int xprev;

	  xprev = points[0].x;
	  i = 1;
	  do 			/* while i != numSpans */
	    {
	      x = points[i].x;
	      if (xprev > x) 
		{
		  /* points[i] is out of order.  Move into proper location. */
		  miIntPoint tpt;
		  unsigned int tw;
		  int k;

		  for (j = 0; x >= points[j].x; j++) 
		    { 
		    }
		  tpt = points[i];
		  tw  = widths[i];
		  for (k = i; k != j; k--) 
		    {
		      points[k] = points[k-1];
		      widths[k] = widths[k-1];
		    }
		  points[j] = tpt;
		  widths[j] = tw;
		  x = points[i].x;
		} /* if out of order */
	      xprev = x;
	      i++;
	    } while (i != numSpans);

	  /* end of insertion sort */
	  return;
	}

      /* Choose partition element, stick in location 0 */
      m = numSpans / 2;
      if (points[m].x > points[0].x)
	ExchangeSpans(m, 0);
      if (points[m].x > points[numSpans-1].x) 
	ExchangeSpans(m, numSpans-1);
      if (points[m].x > points[0].x)
	ExchangeSpans(m, 0);
      x = points[0].x;

      /* Partition array */
      i = 0;
      j = numSpans;
      do 
	{
	  r = &(points[i]);
	  do 
	    {
	      r++;
	      i++;
	    } 
	  while (i != numSpans && r->x < x)
	    ;
	  r = &(points[j]);
	  do 
	    {
	      r--;
	      j--;
	    } 
	  while (x < r->x);
	  if (i < j) ExchangeSpans(i, j);
	} 
      while (i < j);

      /* Move partition element back to middle */
      ExchangeSpans(0, j);

      /* Recurse */
      if (numSpans-j-1 > 1)
	QuickSortSpansX (&points[j+1], &widths[j+1], numSpans-j-1);
      numSpans = j;
    } while (numSpans > 1);

}

/* Uniquify the spans in a Spans, and stash them into the previously
   allocated arrays newPoints and newWidths.  Returns the number of unique
   spans.  Called only if numSpans > 1. */
static int 
#ifdef _HAVE_PROTOS
UniquifySpansX (const Spans *spans, miIntPoint *newPoints, unsigned int *newWidths)
#else
UniquifySpansX (spans, newPoints, newWidths)
     const Spans *spans;
     miIntPoint *newPoints;
     unsigned int *newWidths;
#endif
{
  int newx1, newx2, oldpt, i, y;
  miIntPoint   *oldPoints;
  unsigned int *oldWidths;
  unsigned int *startNewWidths;

  startNewWidths = newWidths;
  oldPoints = spans->points;
  oldWidths = spans->widths;
  y = oldPoints->y;
  newx1 = oldPoints->x;
  newx2 = newx1 + *oldWidths;

  for (i = spans->count - 1; i != 0; i--) 
    {
      oldPoints++;
      oldWidths++;
      oldpt = oldPoints->x;
      if (oldpt > newx2) 
	{
	  /* Write current span, start a new one */
	  newPoints->x = newx1;
	  newPoints->y = y;
	  *newWidths = newx2 - newx1;
	  newPoints++;
	  newWidths++;
	  newx1 = oldpt;
	  newx2 = oldpt + *oldWidths;
	} 
      else 
	{
	  /* extend current span, if old extends beyond new */
	  oldpt = oldpt + *oldWidths;
	  if (oldpt > newx2) newx2 = oldpt;
	}
    }

  /* Write final span */
  newPoints->x = newx1;
  *newWidths = newx2 - newx1;
  newPoints->y = y;

  return (newWidths - startNewWidths) + 1;
}

/* Free the array that is the allocated part of a SpanGroup.  Does not free
   the point and width arrays that are part of each Spans; that must be
   done first. */
void
#ifdef _HAVE_PROTOS
miFreeSpanGroup (SpanGroup *spanGroup)
#else
miFreeSpanGroup (spanGroup)
     SpanGroup *spanGroup;
#endif
{
  if (spanGroup->group != NULL) 
    free (spanGroup->group);
}

/* Paint a SpanGroup, insuring that each pixel is painted at most once;
   also deallocate each Spans, sort of. */
void
#ifdef _HAVE_PROTOS
miFillUniqueSpanGroup (miGC *pGC, SpanGroup *spanGroup)
#else
miFillUniqueSpanGroup (pGC, spanGroup)
     miGC *pGC;
     SpanGroup *spanGroup;
#endif
{
  int    i;
  Spans  *spans;
  Spans  *yspans;
  int    *ysizes;
  int    ymin, ylength;

  /* the Spans used in the single call we'll make to miFillSpans */
  miIntPoint   *points;
  unsigned int *widths;
  int	 count;

  if (spanGroup->count == 0) 
    return;
  if (spanGroup->count == 1) 
    /* only a single Spans */
    {
      /* spans in the single Spans should already be sorted, unique */
      spans = spanGroup->group;
      miFillSpans (pGC, spans->count, spans->points, spans->widths, true);
      free (spans->points);
      free (spans->widths);
    }
  else
    /* more than one Spans, must do stuff */
    {
      /* Yuck.  Gross.  Radix sort into y buckets, then sort x and uniquify */
      /* This seems to be the fastest thing to do.  I've tried sorting on
	 both x and y at the same time rather than creating into all those
	 y buckets, but it was somewhat slower. */

      ymin    = spanGroup->ymin;
      ylength = spanGroup->ymax - ymin + 1;

      /* allocate Spans's for y buckets (one Spans for every scanline) */
      yspans = (Spans *)mi_xmalloc(ylength * sizeof(Spans));
      ysizes = (int *)mi_xmalloc(ylength * sizeof (int));
      for (i = 0; i != ylength; i++) 
	{
	  ysizes[i]        = 0;
	  yspans[i].count  = 0;
	  yspans[i].points = NULL;
	  yspans[i].widths = NULL;
	}

      /* Go through every single span and put it into the correct bucket */
      count = 0;
      for (i = 0, spans = spanGroup->group; 
	   i != spanGroup->count; i++, spans++) 
	{
	  int j, index;

	  for (j = 0, points = spans->points, widths = spans->widths;
	       j != spans->count; j++, points++, widths++) 
	    {
	      index = points->y - ymin;
	      if (index >= 0 && index < ylength) 
		{
		  Spans *newspans = &(yspans[index]);

		  if (newspans->count == ysizes[index]) 
		    /* expand arrays by reallocating */
		    {
		      ysizes[index] = (ysizes[index] + 8) * 2;
		      newspans->points
			= (miIntPoint *)mi_xrealloc(newspans->points,
					   ysizes[index] * sizeof(miIntPoint));
		      newspans->widths
			= (unsigned int *)mi_xrealloc(newspans->widths,
				         ysizes[index] * sizeof(unsigned int));
		    }
		  newspans->points[newspans->count] = *points;
		  newspans->widths[newspans->count] = *widths;
		  (newspans->count)++;
		} /* if y value of span in range */
	    } /* for j through spans */

	  count += spans->count;
	  free (spans->points);
	  spans->points = NULL;
	  free (spans->widths);
	  spans->widths = NULL;
	} /* for i through Spans */

      /* Now sort by x and uniquify each bucket into the final array */
      points = (miIntPoint *)mi_xmalloc(count * sizeof(miIntPoint));
      widths = (unsigned int *)mi_xmalloc(count * sizeof(unsigned int));
      count = 0;
      for (i = 0; i != ylength; i++) 
	{
	  int ycount = yspans[i].count;

	  if (ycount > 0) 
	    {
	      if (ycount > 1) 
		/* sort the >1 spans at this value of y */
		{
		  QuickSortSpansX (yspans[i].points, yspans[i].widths, ycount);
		  count += UniquifySpansX
		    (&(yspans[i]), &(points[count]), &(widths[count]));
		} 
	      else 
		/* just a single span at this value of y */
		{
		  points[count] = yspans[i].points[0];
		  widths[count] = yspans[i].widths[0];
		  count++;
		}
	      free (yspans[i].points);
	      free (yspans[i].widths);
	    }
	}
      
      /* call miFillSpans, with our single constructed Spans as argument */
      miFillSpans (pGC, count, points, widths, true);
      /* free temporary Spans */
      free (points);
      free (widths);
      free (yspans);
      free (ysizes);
    }
  
  /* partially deallocate SpanGroup structure (don't deallocate array?) */
  spanGroup->count = 0;
  spanGroup->ymin = INT_MAX;
  spanGroup->ymax = INT_MIN;
}

/* Paint a SpanGroup, with overlap allowed; also deallocate each Spans. */
void 
#ifdef _HAVE_PROTOS
miFillSpanGroup (miGC *pGC, SpanGroup *spanGroup)
#else
miFillSpanGroup (pGC, spanGroup)
     miGC *pGC;
     SpanGroup *spanGroup;
#endif
{
  int    i;
  Spans  *spans;

  /* loop through Spans's in SpanGroup, freeing point and width array for
     each Spans [but not the Spans itself??] */
  for (i = 0, spans = spanGroup->group; i != spanGroup->count; i++, spans++) 
    {
      /* invoke drawing routine on each Spans */
      miFillSpans (pGC, spans->count, spans->points, spans->widths, true);
      free (spans->points);
      free (spans->widths);
    }

  /* partially deallocate SpanGroup structure (don't deallocate array?) */
  spanGroup->count = 0;
  spanGroup->ymin = INT_MAX;
  spanGroup->ymax = INT_MIN;
}
