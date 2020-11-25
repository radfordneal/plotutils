#include "sys-defines.h"
#include "extern.h"

/* Authors: Keith Packard and Bob Scheifler */
/* Warning: this code is toxic, do not dally very long here. */

#include "g_mi.h"
#include "g_mifply.h"
#include "g_mifllarc.h"

/* undefine if cbrt(), a fast cube root function for non-negative
   arguments, is available */
#define cbrt(x) pow((x), 1.0/3.0)

/* undefine if hypot is available (it's X_OPEN, but not ANSI or POSIX) */
#define hypot(x, y) sqrt((x)*(x) + (y)*(y))

/*
 * Some interesting semantic interpretation of the protocol:
 *
 * Self intersecting arcs (i.e. those spanning 360 degrees) never join with
 * other arcs, and are drawn without caps (unless on/off dashed, in which
 * case each dash segment is capped, except when the last segment meets the
 * first segment, when no caps are drawn).
 *
 * Double dash arcs are drawn in two parts, first the odd dashes (drawn in
 * background) then the even dashes (drawn in foreground).  This means that
 * overlapping sections of foreground/background are drawn twice, first in
 * background then in foreground.  This is the same way the wide-line code
 * works and should be "fixed".
 */

typedef struct
{
  int	arcIndex0, arcIndex1;
  int	phase0, phase1;
  int	end0, end1;
} miArcJoinStruct;

typedef struct
{
  int	arcIndex;
  int	end;		
} miArcCapStruct;

typedef struct
{
  SppPoint	clock;
  SppPoint	center;
  SppPoint	counterClock;
} miArcFace;

typedef struct
{
  miArc		arc;
  bool		render;		/* render after drawing? */
  int		join;		/* related join */
  int		cap;		/* related cap */
  bool		selfJoin;	/* final dash meets first dash? */
  miArcFace	bounds[2];	/* left face and right face */
  double	x0, y0, x1, y1;
} miArcData;

/*
 * This is an entire sequence of arcs, computed and categorized according
 * to operation.  miDashArcs generates either one or two of these.
 */

typedef struct
{
  int			narcs;
  miArcData		*arcs;
  int			ncaps;
  miArcCapStruct	*caps;
  int			njoins;
  miArcJoinStruct	*joins;
} miPolyArcs;

/* In miComputeWideEllipse we compute and cache the list of spans
   associated with a wide ellipse, so that we can retrieve them later by
   keying on line width, ellipse width, ellipse height. */

typedef struct 
{
  int lx, rx;
  int lw, rw;
} miArcSpan;

/* `value' part of each cache record */
typedef struct 
{
  miArcSpan *spans;		/* list of miArcSpan structures */
  int count1, count2, k;
  bool top, bot, hole;
} miArcSpanData;

/* cache record type (key/value) */
typedef struct 
{
  unsigned long lrustamp;	/* timestamp (time of most recent retrieval) */
  unsigned int lw;		/* line width */
  unsigned int width, height;	/* ellipse width, height */
  miArcSpanData *spdata;	/* `value' part of record */
} arcCacheStruct;

/* Static (global) variables that define the cache. */
#define CACHESIZE 25
static arcCacheStruct arcCache[CACHESIZE];
static unsigned long lrustamp;	/* clock, for timestamping cache records */
static arcCacheStruct *lastCacheHit = &arcCache[0];

/* Structure used by a sub-module that computes arc lengths via a
   polygonal approximation to the arc. */
#define DASH_MAP_SIZE 91
typedef struct 
{
  double	map[DASH_MAP_SIZE];
} dashMap;

/* misc. */
#define todeg(xAngle)	(((double) (xAngle)) / 64.0)

#define RIGHT_END	0
#define LEFT_END	1

#define CUBE_ROOT_2	1.2599210498948732038115849718451499938964
#define CUBE_ROOT_4	1.5874010519681993173435330390930175781250

/* forward references */
static double angleBetween ____P((SppPoint center, SppPoint point1, SppPoint point2));
static double miDasin ____P((double v));
static double miDatan2 ____P((double dy, double dx));
static double miDcos ____P((double a));
static double miDsin ____P((double a));
static int computeAngleFromPath ____P((int startAngle, int endAngle, const dashMap *map, int *lenp, bool backwards));
static int miGetArcPts ____P((const SppArc *parc, int cpt, SppPoint **ppPts));
static miArcData * addArc ____P((miArcData **arcsp, int *narcsp, int *sizep, const miArc *xarc));
static miArcSpanData * miComputeWideEllipse ____P((unsigned int lw, const miArc *parc, bool *mustFree));
static miPolyArcs * miComputeArcs ____P((const miArc *parcs, int narcs, miGC *pGC));
static void addCap ____P((miArcCapStruct **capsp, int *ncapsp, int *sizep, int end, int arcIndex));
static void addJoin ____P((miArcJoinStruct **joinsp, int *njoinsp, int *sizep, int end0, int index0, int phase0, int end1, int index1, int phase1));
static void computeDashMap ____P((const miArc *arcp, dashMap *map));
static void drawArc ____P((const miArc *tarc, unsigned int l, int a0, int a1, miArcFace *right, miArcFace *left));
static void drawZeroArc ____P((miGC *pGC, const miArc *tarc, unsigned int lw, miArcFace *left, miArcFace *right));
static void fillSpans ____P((miGC *pGC));
static void miArcCap ____P((miGC *pGC, const miArcFace *pFace, int end, int xOrg, int yOrg, double xFtrans, double yFtrans));
static void miArcJoin ____P((miGC *pGC, const miArcFace *pLeft, const miArcFace *pRight, int xOrgLeft, int yOrgLeft, double xFtransLeft, double yFtransLeft, int xOrgRight, int yOrgRight, double xFtransRight, double yFtransRight));
static void miArcSegment ____P((miGC *pGC, miArc tarc, miArcFace *right, miArcFace *left));
static void miComputeCircleSpans ____P((unsigned int lw, const miArc *parc, miArcSpanData *spdata));
static void miComputeEllipseSpans ____P((unsigned int lw, const miArc *parc, miArcSpanData *spdata));
static void miFillWideEllipse ____P((miGC *pGC, const miArc *parc));
static void miFreeArcs ____P((miPolyArcs *arcs, miGC *pGC));
static void miRoundCap ____P((miGC *pGC, SppPoint pCenter, SppPoint pEnd, SppPoint pCorner, SppPoint pOtherCorner, int fLineEnd, int xOrg, int yOrg, double xFtrans, double yFtrans));
static void translateBounds ____P((miArcFace *b, int x, int y, double fx, double fy));

/*
 * miPolyArc strategy:
 *
 * If arc is zero width and solid, we don't have to worry about the join
 * style.  For wide solid circles, we use a fast integer algorithm.  For
 * wide solid ellipses, we use special case floating point code.
 */
void
#ifdef _HAVE_PROTOS
miPolyArc (miGC *pGC, int narcs, const miArc *parcs)
#else
miPolyArc (pGC, narcs, parcs)
     miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  int           i;
  const miArc   *parc;
  int           width;
  miPixel       fg, bg;
  miPolyArcs    *polyArcs;
  int           cap[2], join[2];
  int           iphase;

  width = pGC->lineWidth;

  if (width == 0 && pGC->lineStyle == miLineSolid)
    /* zero-width solid arc */
    {
      for (i = narcs, parc = parcs; --i >= 0; parc++)
	/* draw a single arc segment (to memory) */
	miArcSegment (pGC, *parc, (miArcFace *)NULL, (miArcFace *)NULL);
      /* really draw the in-core arc segments */
      fillSpans (pGC);
    }
  else 
    {
      if ((pGC->lineStyle == miLineSolid) && narcs)
	{
	  /* fill any complete wide ellipses */
	  while (parcs->width && parcs->height
		 && (parcs->angle2 >= FULLCIRCLE ||
		     parcs->angle2 <= -FULLCIRCLE))
	    {
	      miFillWideEllipse (pGC, parcs);
	      if (!--narcs)
		return;
	      parcs++;
	    }
	}

      /* have a (possibly dashed) incomplete ellipse to draw */

      fg = pGC->fgPixel;
      bg = pGC->bgPixel;
      if ((pGC->fillStyle == miFillTiled) ||
	  (pGC->fillStyle == miFillOpaqueStippled))
	bg = fg;	/* the protocol sez these don't cause color changes */

      /* compute arc segments in the incomplete ellipse;
	 they will need to be freed with miFreeArcs() */
      polyArcs = miComputeArcs (parcs, narcs, pGC);

      /* do odd and even segments (i.e. dashes) separately */
      cap[0] = cap[1] = 0;
      join[0] = join[1] = 0;
      for (iphase = ((pGC->lineStyle == miLineDoubleDash) ? 1 : 0);
	   iphase >= 0;
	   iphase--)
	{
	  if (iphase == 1) 
	    pGC->fgPixel = bg;
	  else if (pGC->lineStyle == miLineDoubleDash) 
	    pGC->fgPixel = fg;

	  for (i = 0; i < polyArcs[iphase].narcs; i++) 
	    {
	      miArcData *arcData;
	      
	      /* draw a single arc segment (to memory) */
	      arcData = &polyArcs[iphase].arcs[i];
	      miArcSegment (pGC, arcData->arc,
			   &arcData->bounds[RIGHT_END],
			   &arcData->bounds[LEFT_END]);
	      if (polyArcs[iphase].arcs[i].render) 
		/* really draw it */
		{
		  fillSpans (pGC);
		  /*
		   * don't cap self-joining arcs
		   */
		  if (polyArcs[iphase].arcs[i].selfJoin
		      && cap[iphase] < polyArcs[iphase].arcs[i].cap)
		    cap[iphase]++;
		  while (cap[iphase] < polyArcs[iphase].arcs[i].cap) 
		    {
		      int	arcIndex, end;
		      miArcData *arcData0;
									     
		      arcIndex = polyArcs[iphase].caps[cap[iphase]].arcIndex;
		      end = polyArcs[iphase].caps[cap[iphase]].end;
		      arcData0 = &polyArcs[iphase].arcs[arcIndex];
		      /* draw cap */
		      miArcCap (pGC,
				&arcData0->bounds[end], end,
				arcData0->arc.x, arcData0->arc.y,
				(double)(0.5 * arcData0->arc.width),
				(double)(0.5 * arcData0->arc.height));
		      ++cap[iphase];
		    }
		  while (join[iphase] < polyArcs[iphase].arcs[i].join) 
		    {
		      int	arcIndex0, arcIndex1, end0, end1;
		      int	phase0, phase1;
		      miArcData *arcData0, *arcData1;
		      miArcJoinStruct *joinp;

		      joinp = &polyArcs[iphase].joins[join[iphase]];
		      arcIndex0 = joinp->arcIndex0;
		      end0 = joinp->end0;
		      arcIndex1 = joinp->arcIndex1;
		      end1 = joinp->end1;
		      phase0 = joinp->phase0;
		      phase1 = joinp->phase1;
		      arcData0 = &polyArcs[phase0].arcs[arcIndex0];
		      arcData1 = &polyArcs[phase1].arcs[arcIndex1];
		      /* draw join */
		      miArcJoin (pGC,
				 &arcData0->bounds[end0],
				 &arcData1->bounds[end1],
				 arcData0->arc.x, arcData0->arc.y,
				 (double) (0.5 * arcData0->arc.width),
				 (double) (0.5 * arcData0->arc.height),
				 arcData1->arc.x, arcData1->arc.y,
				 (double) (0.5 * arcData1->arc.width),
				 (double) (0.5 * arcData1->arc.height));
		      ++join[iphase];
		    }
		}
	    }
	}
      /* free computed arc segments */
      miFreeArcs(polyArcs, pGC);
    }
}

/* Draw a single arc segment using the arc spans generation routines.
 * Right and left faces must be specified (they're usually computed by
 * miComputeArcs()).
 */
static void
#ifdef _HAVE_PROTOS
miArcSegment (miGC *pGC, miArc tarc, miArcFace *right, miArcFace *left)
#else
miArcSegment (pGC, tarc, right, left)
     miGC *pGC;
     miArc tarc;
     miArcFace *right, *left;
#endif
{
  unsigned int l = pGC->lineWidth;
  int a0, a1, startAngle, endAngle;
  miArcFace *temp;

  if (l == 0)
    l = 1;
  
  if (tarc.width == 0 || tarc.height == 0) 
    {
      /* degenerate case, either horizontal or vertical arc */
      drawZeroArc (pGC, &tarc, l, left, right);
      return;
    }
  
  a0 = tarc.angle1;
  a1 = tarc.angle2;
  if (a1 > FULLCIRCLE)
    a1 = FULLCIRCLE;
  else if (a1 < -FULLCIRCLE)
    a1 = -FULLCIRCLE;
  if (a1 < 0) 
    {
      startAngle = a0 + a1;
      endAngle = a0;
      temp = right;
      right = left;
      left = temp;
    } 
  else 
    {
      startAngle = a0;
      endAngle = a0 + a1;
    }
  /*
   * bounds check the two angles
   */
  if (startAngle < 0)
    startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
  if (startAngle >= FULLCIRCLE)
    startAngle = startAngle % FULLCIRCLE;
  if (endAngle < 0)
    endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
  if (endAngle > FULLCIRCLE)
    endAngle = (endAngle-1) % FULLCIRCLE + 1;
  if ((startAngle == endAngle) && a1) 
    {
      startAngle = 0;
      endAngle = FULLCIRCLE;
    }
  
  /* draw the arc to memory (will need to drawn `for real' by fillSpans()) */
  drawArc (&tarc, l, startAngle, endAngle, right, left);
}


/*
Three equations combine to describe the boundaries of the arc

x^2/w^2 + y^2/h^2 = 1			ellipse itself
(X-x)^2 + (Y-y)^2 = r^2			circle at (x, y) on the ellipse
(Y-y) = (X-x)*w^2*y/(h^2*x)		normal at (x, y) on the ellipse

These lead to a quartic relating Y and y

y^4 - (2Y)y^3 + (Y^2 + (h^4 - w^2*r^2)/(w^2 - h^2))y^2
    - (2Y*h^4/(w^2 - h^2))y + (Y^2*h^4)/(w^2 - h^2) = 0

The reducible cubic obtained from this quartic is

z^3 - (3N)z^2 - 2V = 0

where

N = (Y^2 + (h^4 - w^2*r^2/(w^2 - h^2)))/6
V = w^2*r^2*Y^2*h^4/(4 *(w^2 - h^2)^2)

Let

t = z - N
p = -N^2
q = -N^3 - V

Then we get

t^3 + 3pt + 2q = 0

The discriminant of this cubic is

D = q^2 + p^3

When D > 0, a real root is obtained as

z = N + cbrt(-q+sqrt(D)) + cbrt(-q-sqrt(D))

When D < 0, a real root is obtained as

z = N - 2m*cos(acos(-q/m^3)/3)

where

m = sqrt(|p|) * sign(q)

Given a real root Z of the cubic, the roots of the quartic are the roots
of the two quadratics

y^2 + ((b+A)/2)y + (Z + (bZ - d)/A) = 0

where 

A = +/- sqrt(8Z + b^2 - 4c)
b, c, d are the cubic, quadratic, and linear coefficients of the quartic

Some experimentation is then required to determine which solutions
correspond to the inner and outer boundaries.

*/


/* Draw a wide ellipse (called by miPolyArc if angle is at least 360
   degrees). */
static void
#ifdef _HAVE_PROTOS
miFillWideEllipse (miGC *pGC, const miArc *parc)
#else
miFillWideEllipse (pGC, parc)
     miGC *pGC;
     const miArc *parc;
#endif
{
  miIntPoint *points, *pts;
  unsigned int *widths, *wids;
  miArcSpanData *spdata;
  bool mustFree;
  miArcSpan *span;
  int xorg, yorgu, yorgl;
  int n;

  yorgu = parc->height + pGC->lineWidth;
  n = 2 * yorgu;
  widths = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * n);
  points = (miIntPoint *)mi_xmalloc (sizeof(miIntPoint) * n);

  spdata = miComputeWideEllipse (pGC->lineWidth, parc, &mustFree);
  if (!spdata)
    {
      free (widths);
      free (points);
      return;
    }
  pts = points;
  wids = widths;
  span = spdata->spans;
  xorg = parc->x + (int)(parc->width >> 1);
  yorgu = parc->y + (int)(parc->height >> 1);
  yorgl = yorgu + (parc->height & 1);
  yorgu -= spdata->k;
  yorgl += spdata->k;
  if (spdata->top)
    {
      pts->x = xorg;
      pts->y = yorgu - 1;
      pts++;
      *wids++ = 1;
      span++;
    }
  for (n = spdata->count1; --n >= 0; )
    {
      pts[0].x = xorg + span->lx;
      pts[0].y = yorgu;
      wids[0] = span->lw;
      pts[1].x = pts[0].x;
      pts[1].y = yorgl;
      wids[1] = wids[0];
      yorgu++;
      yorgl--;
      pts += 2;
      wids += 2;
      span++;
    }
  if (spdata->hole)
    {
      pts[0].x = xorg;
      pts[0].y = yorgl;
      wids[0] = 1;
      pts++;
      wids++;
    }
  for (n = spdata->count2; --n >= 0; )
    {
      pts[0].x = xorg + span->lx;
      pts[0].y = yorgu;
      wids[0] = span->lw;
      pts[1].x = xorg + span->rx;
      pts[1].y = pts[0].y;
      wids[1] = span->rw;
      pts[2].x = pts[0].x;
      pts[2].y = yorgl;
      wids[2] = wids[0];
      pts[3].x = pts[1].x;
      pts[3].y = pts[2].y;
      wids[3] = wids[1];
      yorgu++;
      yorgl--;
      pts += 4;
      wids += 4;
      span++;
    }
  if (spdata->bot)
    {
      if (span->rw <= 0)
	{
	  pts[0].x = xorg + span->lx;
	  pts[0].y = yorgu;
	  wids[0] = span->lw;
	  pts++;
	  wids++;
	}	
      else
	{
	  pts[0].x = xorg + span->lx;
	  pts[0].y = yorgu;
	  wids[0] = span->lw;
	  pts[1].x = xorg + span->rx;
	  pts[1].y = pts[0].y;
	  wids[1] = span->rw;
	  pts += 2;
	  wids += 2;
	}
    }
  if (mustFree)
    free (spdata);
  miFillSpans (pGC, pts - points, points, widths, false);

  free (points);
  free (widths);
}

/* Compute the spans that make up a wide ellipse.  (We cache them for
   speedy later retrieval.) */
static miArcSpanData *
#ifdef _HAVE_PROTOS
miComputeWideEllipse (unsigned int lw, const miArc *parc, bool *mustFree)
#else
miComputeWideEllipse (lw, parc, mustFree)
     unsigned int lw;
     const miArc *parc;
     bool *mustFree;
#endif
{
  miArcSpanData *spdata;
  arcCacheStruct *cent, *lruent;
  int k;
  arcCacheStruct fakeent;

  if (lw == 0)
    lw = 1;
  if (parc->height <= 1500)
    {
      *mustFree = false;
      cent = lastCacheHit;
      if (cent->lw == lw 
	  && cent->width == parc->width && cent->height == parc->height)
	/* last hit is still valid; won't need to search */
	{
	  cent->lrustamp = ++lrustamp; /* hit again; do timestamp, bump time */
	  return cent->spdata;
	}
      /* search cache (an array) */
      lruent = &arcCache[0];
      for (k = CACHESIZE, cent = lruent; --k >= 0; cent++)
	{
	  if (cent->lw == lw 
	      && cent->width == parc->width && cent->height == parc->height)
	    /* a hit; success */
	    {
	      cent->lrustamp = ++lrustamp; /* do timestamp, bump time */
	      lastCacheHit = cent;
	      return cent->spdata;
	    }
	  /* keep track of least recently used record */
	  if (cent->lrustamp < lruent->lrustamp)
	    lruent = cent;
	}
    } 
  else /* height > 1500 */
    {
      lruent = &fakeent;	/* _very_ fake; automatic variable */
      lruent->spdata = NULL;
      *mustFree = true;
    }

  /* boot least-recently used record out of cache, make new one */
  k = (int)(parc->height >> 1) + (int)((lw - 1) >> 1);
  spdata = lruent->spdata;
  if (spdata == NULL || spdata->k != k)
    {
      if (spdata)
	free (spdata);
      spdata = (miArcSpanData *)mi_xmalloc (sizeof(miArcSpanData) +
					       sizeof(miArcSpan) * (k + 2));
      lruent->spdata = spdata;
      spdata->spans = (miArcSpan *)(spdata + 1);
      spdata->k = k;
    }
  spdata->top = !(lw & 1) && !(parc->width & 1) ? true : false;
  spdata->bot = !(parc->height & 1) ? true : false;
  lruent->lrustamp = ++lrustamp; /* set timestamp, bump clock */
  lruent->lw = lw;
  lruent->width = parc->width;
  lruent->height = parc->height;
  if (lruent != &fakeent)
    lastCacheHit = lruent;

  /* compute spans (so that they'll be part of new record) */
  if (parc->width == parc->height)
    miComputeCircleSpans (lw, parc, spdata);
  else
    miComputeEllipseSpans (lw, parc, spdata);
  return spdata;
}

#if 0
/*
 * Not currently called, but could be.
 */
static void
miFreeArcCache ()
{
  int k;
  arcCacheStruct *cent;

  for (k = CACHESIZE, cent = &arcCache[0]; --k >= 0; cent++)
    {
      if (cent->spdata)
	{
	  cent->lrustamp = 0;	/* reset timestamp on record */
	  cent->lw = 0;
	  free (cent->spdata);
	  cent->spdata = NULL;
	}
    }
  lrustamp = 0;			/* reset global clock */
}
#endif

/* Compute the spans that make up a wide circular arc.  On entry, lw >= 1 */
static void
#ifdef _HAVE_PROTOS
miComputeCircleSpans (unsigned int lw, const miArc *parc, miArcSpanData *spdata)
#else
miComputeCircleSpans (lw, parc, spdata)
     unsigned int lw;
     const miArc *parc;
     miArcSpanData *spdata;
#endif
{
  miArcSpan *span;
  int doinner;
  int x, y, e;
  int xk, yk, xm, ym, dx, dy;
  int slw, inslw;
  int inx = 0, iny, ine = 0;
  int inxk = 0, inyk = 0, inxm = 0, inym = 0;

  doinner = -(int)lw;
  slw = (int)parc->width - doinner;
  y = (int)(parc->height >> 1);
  dy = parc->height & 1;
  dx = 1 - dy;
  MIWIDEARCSETUP(x, y, dy, slw, e, xk, xm, yk, ym);
  inslw = (int)parc->width + doinner;
  if (inslw > 0)
    {
      spdata->hole = spdata->top;
      MIWIDEARCSETUP(inx, iny, dy, inslw, ine, inxk, inxm, inyk, inym);
    }
  else
    {
      spdata->hole = false;
      doinner = -y;
    }
  spdata->count1 = -doinner - (spdata->top ? 1 : 0);
  spdata->count2 = y + doinner;
  span = spdata->spans;
  while (y)
    {
      MIFILLARCSTEP(slw);
      span->lx = dy - x;
      if (++doinner <= 0)
 	{
	  span->lw = slw;
	  span->rx = 0;
	  span->rw = span->lx + slw;
	}
      else
	{
	  MIFILLINARCSTEP(inslw);
	  span->lw = x - inx;
	  span->rx = dy - inx + inslw;
	  span->rw = inx - x + slw - inslw;
	}
      span++;
    }
  if (spdata->bot)
    {
      if (spdata->count2)
	spdata->count2--;
      else
	{
	  if (lw > parc->height)
	    span[-1].rx = span[-1].rw = -(((int)lw - (int)parc->height) >> 1);
	  else
	    span[-1].rw = 0;
	  spdata->count1--;
	}
    }
}

/* Compute the spans that make up a wide elliptic arc.  On entry, lw >= 1 */
static void
#ifdef _HAVE_PROTOS
miComputeEllipseSpans (unsigned int lw, const miArc *parc, miArcSpanData *spdata)
#else
miComputeEllipseSpans (lw, parc, spdata)
     unsigned int lw;
     const miArc *parc;
     miArcSpanData *spdata;
#endif
{
  miArcSpan *span;
  double w, h, r, xorg;
  double Hs, Hf, WH, K, Vk, Nk, Fk, Vr, N, Nc, Z, rs;
  double A, T, b, d, x, y, t, inx, outx = 0, hepp, hepm;
  int flip, solution;
  
  w = 0.5 * parc->width;
  h = 0.5 * parc->height;
  r = 0.5 * lw;
  rs = r * r;
  Hs = h * h;
  WH = w * w - Hs;
  Nk = w * r;
  Vk = (Nk * Hs) / (WH + WH);
  Hf = Hs * Hs;
  Nk = (Hf - Nk * Nk) / WH;
  Fk = Hf / WH;
  hepp = h + EPSILON;
  hepm = h - EPSILON;
  K = h + ((lw - 1) >> 1);
  span = spdata->spans;
  if (parc->width & 1)
    xorg = .5;
  else
    xorg = 0.0;
  if (spdata->top)
    {
      span->lx = 0;
      span->lw = 1;
      span++;
    }
  spdata->count1 = 0;
  spdata->count2 = 0;
  spdata->hole = ((spdata->top 
		   && parc->height * lw <= parc->width * parc->width
		   && lw < parc->height) ? true : false);
  for (; K > 0.0; K -= 1.0)
    {
      N = (K * K + Nk) / 6.0;
      Nc = N * N * N;
      Vr = Vk * K;
      t = Nc + Vr * Vr;
      d = Nc + t;
      if (d < 0.0) 
	{
	  d = Nc;
	  b = N;
	  if ( (b < 0.0) == (t < 0.0) )
	    {
	      b = -b;
	      d = -d;
	    }
	  Z = N - 2.0 * b * cos(acos(-t / d) / 3.0);
	  if ( (Z < 0.0) == (Vr < 0.0) )
	    flip = 2;
	  else
	    flip = 1;
	}
      else
	{
	  d = Vr * sqrt(d);
	  Z = N + cbrt(t + d) + cbrt(t - d);
	  flip = 0;
	}
      A = sqrt((Z + Z) - Nk);
      T = (Fk - Z) * K / A;
      inx = 0.0;
      solution = false;
      b = -A + K;
      d = b * b - 4 * (Z + T);
      if (d >= 0)
	{
	  d = sqrt(d);
	  y = (b + d) / 2;
	  if ((y >= 0.0) && (y < hepp))
	    {
	      solution = true;
	      if (y > hepm)
		y = h;
	      t = y / h;
	      x = w * sqrt(1 - (t * t));
	      t = K - y;
	      t = sqrt(rs - (t * t));
	      if (flip == 2)
		inx = x - t;
	      else
		outx = x + t;
	    }
	}
      b = A + K;
      d = b * b - 4 * (Z - T);
      /* Because of the large magnitudes involved, we lose enough precision
       * that sometimes we end up with a negative value near the axis, when
       * it should be positive.  This is a workaround.
       */
      if (d < 0 && !solution)
	d = 0.0;
      if (d >= 0) 
	{
	  d = sqrt(d);
	  y = (b + d) / 2;
	  if (y < hepp)
	    {
	      if (y > hepm)
		y = h;
	      t = y / h;
	      x = w * sqrt(1 - (t * t));
	      t = K - y;
	      inx = x - sqrt(rs - (t * t));
	    }
	  y = (b - d) / 2;
	  if (y >= 0.0)
	    {
	      if (y > hepm)
		y = h;
	      t = y / h;
	      x = w * sqrt(1 - (t * t));
	      t = K - y;
	      t = sqrt(rs - (t * t));
	      if (flip == 1)
		inx = x - t;
	      else
		outx = x + t;
	    }
	}
      span->lx = ICEIL(xorg - outx);
      if (inx <= 0.0)
	{
	  spdata->count1++;
	  span->lw = ICEIL(xorg + outx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = -ICEIL(xorg - inx);
	}
      else
	{
	  spdata->count2++;
	  span->lw = ICEIL(xorg - inx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = ICEIL(xorg + outx) - span->rx;
	}
      span++;
    }
  if (spdata->bot)
    {
      outx = w + r;
      if (r >= h && r <= w)
	inx = 0.0;
      else if (Nk < 0.0 && -Nk < Hs)
	{
	  inx = w * sqrt(1 + Nk / Hs) - sqrt(rs + Nk);
	  if (inx > w - r)
	    inx = w - r;
	}
      else
	inx = w - r;
      span->lx = ICEIL(xorg - outx);
      if (inx <= 0.0)
	{
	  span->lw = ICEIL(xorg + outx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = -ICEIL(xorg - inx);
	}
      else
	{
	  span->lw = ICEIL(xorg - inx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = ICEIL(xorg + outx) - span->rx;
	}
    }
  if (spdata->hole)
    {
      span = &spdata->spans[spdata->count1];
      span->lw = -span->lx;
      span->rx = 1;
      span->rw = span->lw;
      spdata->count1--;
      spdata->count2++;
    }
}

/* Compute arc segments (i.e. dashes) in an incomplete ellipse, taking
   account of joins and caps.  They'll need to be freed with miFreeArcs().
   This routine is a bit gory. */
static miPolyArcs *
#ifdef _HAVE_PROTOS
miComputeArcs (const miArc *parcs, int narcs, miGC *pGC)
#else
miComputeArcs (parcs, narcs, pGC)
     const miArc *parcs;
     int narcs;
     miGC *pGC;
#endif
{
  int		isDashed, isDoubleDash;
  int		dashOffset;
  miPolyArcs	*arcs;
  int		start, i, j, k = 0, nexti, nextk = 0;
  int		joinSize[2];
  int		capSize[2];
  int		arcSize[2];
  int		angle2;
  double	a0, a1;
  miArcData	*data;
  miArcData	*arc;
  miArc		xarc;
  int		iphase, prevphase = 0, joinphase;
  bool		arcsJoin;
  bool		selfJoin;
  
  int		iDash = 0, dashRemaining;
  int		iDashStart = 0, dashRemainingStart = 0, iphaseStart;
  int		startAngle, spanAngle, endAngle;
  bool		backwards;
  int		prevDashAngle, dashAngle;
  dashMap	map;
  
  isDashed = !(pGC->lineStyle == miLineSolid);
  isDoubleDash = (pGC->lineStyle == miLineDoubleDash);
  dashOffset = pGC->dashOffset;
  
  data = (miArcData *) mi_xmalloc (narcs * sizeof (miArcData));
  arcs = (miPolyArcs *) mi_xmalloc (sizeof (*arcs) * (isDoubleDash ? 2 : 1));
  for (i = 0; i < narcs; i++) 
    {
      a0 = todeg (parcs[i].angle1);
      angle2 = parcs[i].angle2;
      if (angle2 > FULLCIRCLE)
	angle2 = FULLCIRCLE;
      else if (angle2 < -FULLCIRCLE)
	angle2 = -FULLCIRCLE;
      data[i].selfJoin = ((angle2 == FULLCIRCLE) || (angle2 == -FULLCIRCLE) 
			  ? true : false);
      a1 = todeg (parcs[i].angle1 + angle2);
      data[i].x0 = parcs[i].x + (double) parcs[i].width / 2*(1 + miDcos (a0));
      data[i].y0 = parcs[i].y + (double) parcs[i].height / 2*(1 - miDsin (a0));
      data[i].x1 = parcs[i].x + (double) parcs[i].width / 2*(1 + miDcos (a1));
      data[i].y1 = parcs[i].y + (double) parcs[i].height / 2*(1 - miDsin (a1));
    }
  
  for (iphase = 0; iphase < (isDoubleDash ? 2 : 1); iphase++) 
    {
      arcs[iphase].njoins = 0;
      arcs[iphase].joins = 0;
      joinSize[iphase] = 0;
		
      arcs[iphase].ncaps = 0;
      arcs[iphase].caps = 0;
      capSize[iphase] = 0;
		
      arcs[iphase].narcs = 0;
      arcs[iphase].arcs = 0;
      arcSize[iphase] = 0;
    }
  
  iphase = 0;
  if (isDashed) 
    {
      iDash = 0;
      dashRemaining = pGC->dash[0];
      while (dashOffset > 0) 
	{
	  if (dashOffset >= dashRemaining) 
	    {
	      dashOffset -= dashRemaining;
	      iphase = iphase ? 0 : 1;
	      iDash++;
	      if (iDash == pGC->numInDashList)
		iDash = 0;
	      dashRemaining = pGC->dash[iDash];
	    } 
	  else 
	    {
	      dashRemaining -= dashOffset;
	      dashOffset = 0;
	    }
	}
      iDashStart = iDash;
      dashRemainingStart = dashRemaining;
    }
  iphaseStart = iphase;
  
  for (i = narcs - 1; i >= 0; i--) 
    {
      j = i + 1;
      if (j == narcs)
	j = 0;
      if (data[i].selfJoin || i == j ||
	  (UNEQUAL (data[i].x1, data[j].x0) ||
	   UNEQUAL (data[i].y1, data[j].y0)))
	{
	  if (iphase == 0 || isDoubleDash)
	    addCap (&arcs[iphase].caps, &arcs[iphase].ncaps,
		    &capSize[iphase], RIGHT_END, 0);
	  break;
	}
    }
  start = i + 1;
  if (start == narcs)
    start = 0;
  i = start;
  for (;;) 
    {
      j = i + 1;
      if (j == narcs)
	j = 0;
      nexti = i+1;
      if (nexti == narcs)
	nexti = 0;
      if (isDashed) 
	{
	  /*
	   * precompute an approximation map
	   */
	  computeDashMap (&parcs[i], &map);
	  /*
	   * compute each individual dash segment using the path
	   * length function
	   */
	  startAngle = parcs[i].angle1;
	  spanAngle = parcs[i].angle2;
	  if (spanAngle > FULLCIRCLE)
	    spanAngle = FULLCIRCLE;
	  else if (spanAngle < -FULLCIRCLE)
	    spanAngle = -FULLCIRCLE;
	  if (startAngle < 0)
	    startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
	  if (startAngle >= FULLCIRCLE)
	    startAngle = startAngle % FULLCIRCLE;
	  endAngle = startAngle + spanAngle;
	  backwards = (spanAngle < 0 ? true : false);
	  dashAngle = startAngle;
	  selfJoin = (data[i].selfJoin && (iphase == 0 || isDoubleDash) 
		      ? true : false);
	  
	  /*
	   * add dashed arcs to each bucket
	   */
	  arc = 0;
	  while (dashAngle != endAngle) 
	    {
	      prevDashAngle = dashAngle;
	      dashAngle = computeAngleFromPath (prevDashAngle, endAngle, &map,
						&dashRemaining, backwards);
	      /* avoid troubles with huge arcs and small dashes */
	      if (dashAngle == prevDashAngle) 
		{
		  if (backwards)
		    dashAngle--;
		  else
		    dashAngle++;
		}
	      if (iphase == 0 || isDoubleDash) 
		{
		  xarc = parcs[i];
		  spanAngle = prevDashAngle;
		  if (spanAngle < 0)
		    spanAngle = FULLCIRCLE - (-spanAngle) % FULLCIRCLE;
		  if (spanAngle >= FULLCIRCLE)
		    spanAngle = spanAngle % FULLCIRCLE;
		  xarc.angle1 = spanAngle;
		  spanAngle = dashAngle - prevDashAngle;
		  if (backwards) 
		    {
		      if (dashAngle > prevDashAngle)
			spanAngle = - FULLCIRCLE + spanAngle;
		    } 
		  else 
		    {
		      if (dashAngle < prevDashAngle)
			spanAngle = FULLCIRCLE + spanAngle;
		    }
		  if (spanAngle > FULLCIRCLE)
		    spanAngle = FULLCIRCLE;
		  if (spanAngle < -FULLCIRCLE)
		    spanAngle = -FULLCIRCLE;
		  xarc.angle2 = spanAngle;
		  arc = addArc (&arcs[iphase].arcs, &arcs[iphase].narcs,
				&arcSize[iphase], &xarc);
		  if (!arc)
		    goto arcfail;
		  /*
		   * cap each end of an on/off dash
		   */
		  if (!isDoubleDash) 
		    {
		      if (prevDashAngle != startAngle) 
			{
			  addCap (&arcs[iphase].caps,
				  &arcs[iphase].ncaps,
				  &capSize[iphase], RIGHT_END,
				  arc - arcs[iphase].arcs);
							
			}
		      if (dashAngle != endAngle) 
			{
			  addCap (&arcs[iphase].caps,
				  &arcs[iphase].ncaps,
				  &capSize[iphase], LEFT_END,
				  arc - arcs[iphase].arcs);
			}
		    }
		  arc->cap = arcs[iphase].ncaps;
		  arc->join = arcs[iphase].njoins;
		  arc->render = false;
		  arc->selfJoin = false;
		  if (dashAngle == endAngle)
		    arc->selfJoin = selfJoin;
		}
	      prevphase = iphase;
	      if (dashRemaining <= 0) 
		{
		  ++iDash;
		  if (iDash == pGC->numInDashList)
		    iDash = 0;
		  iphase = iphase ? 0:1;
		  dashRemaining = pGC->dash[iDash];
		}
	    }
	  /*
	   * make sure a place exists for the position data when
	   * drawing a zero-length arc
	   */
	  if (startAngle == endAngle) 
	    {
	      prevphase = iphase;
	      if (!isDoubleDash && iphase == 1)
		prevphase = 0;
	      arc = addArc (&arcs[prevphase].arcs, &arcs[prevphase].narcs,
			    &arcSize[prevphase], &parcs[i]);
	      if (!arc)
		goto arcfail;
	      arc->join = arcs[prevphase].njoins;
	      arc->cap = arcs[prevphase].ncaps;
	      arc->selfJoin = data[i].selfJoin;
	    }
	} 
      else 
	{
	  arc = addArc (&arcs[iphase].arcs, &arcs[iphase].narcs,
			&arcSize[iphase], &parcs[i]);
	  if (!arc)
	    goto arcfail;
	  arc->join = arcs[iphase].njoins;
	  arc->cap = arcs[iphase].ncaps;
	  arc->selfJoin = data[i].selfJoin;
	  prevphase = iphase;
	}
      if (prevphase == 0 || isDoubleDash)
	k = arcs[prevphase].narcs - 1;
      if (iphase == 0 || isDoubleDash)
	nextk = arcs[iphase].narcs;
      if (nexti == start) 
	{
	  nextk = 0;
	  if (isDashed) 
	    {
	      iDash = iDashStart;
	      iphase = iphaseStart;
	      dashRemaining = dashRemainingStart;
	    }
	}
      arcsJoin = (narcs > 1 && i != j 
		  && ISEQUAL (data[i].x1, data[j].x0)
		  && ISEQUAL (data[i].y1, data[j].y0)
		  && data[i].selfJoin == false 
		  && data[j].selfJoin == false) ? true : false;
      if (arc)
	{
	  if (arcsJoin)
	    arc->render = false;
	  else
	    arc->render = true;
	}
      if (arcsJoin
	  && (prevphase == 0 || isDoubleDash) 
	  && (iphase == 0 || isDoubleDash))
	{
	  joinphase = iphase;
	  if (isDoubleDash) 
	    {
	      if (nexti == start)
		joinphase = iphaseStart;
	      /*
	       * if the join is right at the dash,
	       * draw the join in foreground
	       * This is because the foreground
	       * arcs are computed second, the results
	       * of which are needed to draw the join
	       */
	      if (joinphase != prevphase)
		joinphase = 0;
	    }
	  if (joinphase == 0 || isDoubleDash) 
	    {
	      addJoin (&arcs[joinphase].joins,
		       &arcs[joinphase].njoins,
		       &joinSize[joinphase],
		       LEFT_END, k, prevphase,
		       RIGHT_END, nextk, iphase);
	      arc->join = arcs[prevphase].njoins;
	    }
	} 
      else 
	{
	  /*
	   * cap the left end of this arc
	   * unless it joins itself
	   */
	  if ((prevphase == 0 || isDoubleDash)
	      && arc->selfJoin == false)
	    {
	      addCap (&arcs[prevphase].caps, &arcs[prevphase].ncaps,
		      &capSize[prevphase], LEFT_END, k);
	      arc->cap = arcs[prevphase].ncaps;
	    }
	  if (isDashed && arcsJoin == false)
	    {
	      iDash = iDashStart;
	      iphase = iphaseStart;
	      dashRemaining = dashRemainingStart;
	    }
	  nextk = arcs[iphase].narcs;
	  if (nexti == start) 
	    {
	      nextk = 0;
	      iDash = iDashStart;
	      iphase = iphaseStart;
	      dashRemaining = dashRemainingStart;
	    }
	  /*
	   * cap the right end of the next arc.  If the
	   * next arc is actually the first arc, only
	   * cap it if it joins with this arc.  This
	   * case will occur when the final dash segment
	   * of an on/off dash is off.  Of course, this
	   * cap will be drawn at a strange time, but that
	   * hardly matters...
	   */
	  if ((iphase == 0 || isDoubleDash)
	      && (nexti != start || (arcsJoin && isDashed)))
	    addCap (&arcs[iphase].caps, &arcs[iphase].ncaps,
		    &capSize[iphase], RIGHT_END, nextk);
	}
      i = nexti;
      if (i == start)
	break;
    }
  /*
   * make sure the last section is rendered
   */
  for (iphase = 0; iphase < (isDoubleDash ? 2 : 1); iphase++)
    if (arcs[iphase].narcs > 0) 
      {
	arcs[iphase].arcs[arcs[iphase].narcs-1].render = true;
	arcs[iphase].arcs[arcs[iphase].narcs-1].join =
	  arcs[iphase].njoins;
	arcs[iphase].arcs[arcs[iphase].narcs-1].cap =
	  arcs[iphase].ncaps;
      }
  free (data);
  return arcs;

 arcfail:
  miFreeArcs(arcs, pGC);
  free (data);
  return (miPolyArcs *)NULL;
}

/* Free the arc segments (i.e. dashes) for an incomplete ellipse, which
   were computed by miComputeArcs(). */
static void
#ifdef _HAVE_PROTOS
miFreeArcs(miPolyArcs *arcs, miGC *pGC)
#else
miFreeArcs(arcs, pGC)
     miPolyArcs *arcs;
     miGC *pGC;
#endif
{
  int iphase;

  for (iphase = ((pGC->lineStyle == miLineDoubleDash) ? 1 : 0);
       iphase >= 0;
       iphase--)
    {
      if (arcs[iphase].narcs > 0)
	free (arcs[iphase].arcs);
      if (arcs[iphase].njoins > 0)
	free (arcs[iphase].joins);
      if (arcs[iphase].ncaps > 0)
	free (arcs[iphase].caps);
    }
  free (arcs);
}

/* helper function, used by miArcJoin() below */
static double
#ifdef _HAVE_PROTOS
angleBetween (SppPoint center, SppPoint point1, SppPoint point2)
#else
angleBetween (center, point1, point2)
     SppPoint center, point1, point2;
#endif
{
  double	a1, a2, a;
	
  /*
   * reflect from X coordinates back to ellipse
   * coordinates -- y increasing upwards
   */
  a1 = miDatan2 (- (point1.y - center.y), point1.x - center.x);
  a2 = miDatan2 (- (point2.y - center.y), point2.x - center.x);
  a = a2 - a1;
  if (a <= -180.0)
    a += 360.0;
  else if (a > 180.0)
    a -= 360.0;

  return a;
}

/* helper function, used by miArcJoin() below */
static void
#ifdef _HAVE_PROTOS
translateBounds (miArcFace *b, int x, int y, double fx, double fy)
#else
translateBounds (b, x, y, fx, fy)
     miArcFace *b;
     int x, y;
     double fx, fy;
#endif
{
  fx += x;
  fy += y;
  b->clock.x -= fx;
  b->clock.y -= fy;
  b->center.x -= fx;
  b->center.y -= fy;
  b->counterClock.x -= fx;
  b->counterClock.y -= fy;
}

/* Draw a join between two contiguous arcs, by calling miFillSppPoly(). */
static void
#ifdef _HAVE_PROTOS
miArcJoin (miGC *pGC, const miArcFace *pLeft, const miArcFace *pRight, int xOrgLeft, int yOrgLeft, double xFtransLeft, double yFtransLeft, int xOrgRight, int yOrgRight, double xFtransRight, double yFtransRight)
#else
miArcJoin (pGC, pLeft, pRight, xOrgLeft, yOrgLeft, xFtransLeft, yFtransLeft, xOrgRight, yOrgRight, xFtransRight, yFtransRight)
     miGC *pGC;
     const miArcFace *pLeft, *pRight;
     int xOrgLeft, yOrgLeft;
     double xFtransLeft, yFtransLeft;
     int xOrgRight, yOrgRight;
     double xFtransRight, yFtransRight;
#endif
{
  SppPoint	center, corner, otherCorner;
  SppPoint	poly[5], e;
  SppPoint	*pArcPts;
  int		cpt;
  SppArc	arc;
  miArcFace	Right, Left;
  int		polyLen = 0;
  int		xOrg, yOrg;
  double	xFtrans, yFtrans;
  double	a;
  double	ae, ac2, ec2, bc2, de;
  double	width;
  double	halftheta;
	
  xOrg = (xOrgRight + xOrgLeft) / 2;
  yOrg = (yOrgRight + yOrgLeft) / 2;
  xFtrans = (xFtransLeft + xFtransRight) / 2;
  yFtrans = (yFtransLeft + yFtransRight) / 2;
  Right = *pRight;
  translateBounds (&Right, xOrg - xOrgRight, yOrg - yOrgRight,
		   xFtrans - xFtransRight, yFtrans - yFtransRight);
  Left = *pLeft;
  translateBounds (&Left, xOrg - xOrgLeft, yOrg - yOrgLeft,
		   xFtrans - xFtransLeft, yFtrans - yFtransLeft);
  pRight = &Right;
  pLeft = &Left;

  if (pRight->clock.x == pLeft->counterClock.x
      && pRight->clock.y == pLeft->counterClock.y)
    return;

  /* determine corners of cap */
  center = pRight->center;
  if (0 <= (a = angleBetween (center, pRight->clock, pLeft->counterClock))
      && a <= 180.0)
    {
      corner = pRight->clock;
      otherCorner = pLeft->counterClock;
    } 
  else		/* interchange to make a <= 180, we hope */
    {
      a = angleBetween (center, pLeft->clock, pRight->counterClock);
      corner = pLeft->clock;
      otherCorner = pRight->counterClock;
    }

  switch (pGC->joinStyle) 
    {
    case miJoinRound:
      /* add round cap */
      width = (pGC->lineWidth ? pGC->lineWidth : 1);

      arc.x = center.x - width/2;
      arc.y = center.y - width/2;
      arc.width = width;
      arc.height = width;
      arc.angle1 = -miDatan2 (corner.y - center.y, corner.x - center.x);
      arc.angle2 = a;

      pArcPts = (SppPoint *) mi_xmalloc (3 * sizeof (SppPoint));
      pArcPts[0].x = otherCorner.x;
      pArcPts[0].y = otherCorner.y;
      pArcPts[1].x = center.x;
      pArcPts[1].y = center.y;
      pArcPts[2].x = corner.x;
      pArcPts[2].y = corner.y;
      /* convert semicircle to a polyline, and fill */
      if ((cpt = miGetArcPts (&arc, 3, &pArcPts)))
	/* by drawing with miFillSppPoly and setting the endpoints of the
	   arc to be the corners, we ensure that the cap will meet up with
	   the rest of the line */
	miFillSppPoly (pGC, cpt, pArcPts, xOrg, yOrg, xFtrans, yFtrans);
      free (pArcPts);
      return;
    case miJoinMiter:
    default:
      /* miter only if MITERLIMIT * sin(theta/2) >= 1.0,
	 where theta = 180-a is the join angle */
      if ((halftheta = 0.5 * (180.0 - a)) > 0.0
	  && miDsin (halftheta) * pGC->miterLimit >= 1.0)
	/* add parallelogram */
	{
	  poly[0] = corner;
	  poly[1] = center;
	  poly[2] = otherCorner;
	  bc2 = (corner.x - otherCorner.x) * (corner.x - otherCorner.x) +
	    (corner.y - otherCorner.y) * (corner.y - otherCorner.y);
	  ec2 = bc2 / 4;
	  ac2 = (corner.x - center.x) * (corner.x - center.x) +
	    (corner.y - center.y) * (corner.y - center.y);
	  ae = sqrt (ac2 - ec2);
	  de = ec2 / ae;
	  e.x = (corner.x + otherCorner.x) / 2;
	  e.y = (corner.y + otherCorner.y) / 2;
	  poly[3].x = e.x + de * (e.x - center.x) / ae;
	  poly[3].y = e.y + de * (e.y - center.y) / ae;
	  poly[4] = corner;
	  polyLen = 5;
	  break;
	}
      /* FALLTHRU */
    case miJoinBevel:
      /* add triangle */
      poly[0] = corner;
      poly[1] = center;
      poly[2] = otherCorner;
      poly[3] = corner;
      polyLen = 4;
      break;
    }
  miFillSppPoly (pGC, polyLen, poly, xOrg, yOrg, xFtrans, yFtrans);
}

/* Draw a cap on an arc segment, by calling miFillSppPoly(). */
/*ARGSUSED*/
static void
#ifdef _HAVE_PROTOS
miArcCap (miGC *pGC, const miArcFace *pFace, int end, int xOrg, int yOrg, double xFtrans, double yFtrans)
#else
miArcCap (pGC, pFace, end, xOrg, yOrg, xFtrans, yFtrans)
     miGC *pGC;
     const miArcFace *pFace;
     int end;
     int xOrg, yOrg;
     double xFtrans, yFtrans;
#endif
{
  SppPoint corner, otherCorner, center, endPoint, poly[5];

  corner = pFace->clock;
  otherCorner = pFace->counterClock;
  center = pFace->center;
  switch (pGC->capStyle) 
    {
    case miCapButt:		/* do nothing */
      break;
    case miCapProjecting:
      poly[0].x = otherCorner.x;
      poly[0].y = otherCorner.y;
      poly[1].x = corner.x;
      poly[1].y = corner.y;
      poly[2].x = corner.x -
	(center.y - corner.y);
      poly[2].y = corner.y +
	(center.x - corner.x);
      poly[3].x = otherCorner.x -
	(otherCorner.y - center.y);
      poly[3].y = otherCorner.y +
	(otherCorner.x - center.x);
      poly[4].x = otherCorner.x;
      poly[4].y = otherCorner.y;
      miFillSppPoly (pGC, 5, poly, xOrg, yOrg, xFtrans, yFtrans);
      break;
    case miCapRound:
      /*
       * miRoundCap() just needs these to be unequal.
       */
      endPoint = center;
      endPoint.x = endPoint.x + 100;
      miRoundCap (pGC, center, endPoint, corner, otherCorner, 0,
		  -xOrg, -yOrg, xFtrans, yFtrans);
      break;
    }
}

/* MIROUNDCAP -- a helper function used by miArcCap() above.
 * Put Rounded cap on end. pCenter is the center of this end of the line
 * pEnd is the center of the other end of the line. pCorner is one of the
 * two corners at this end of the line.  
 * NOTE:  pOtherCorner must be counter-clockwise from pCorner.
 */
/*ARGSUSED*/
static void
#ifdef _HAVE_PROTOS
miRoundCap(miGC *pGC, SppPoint pCenter, SppPoint pEnd, SppPoint pCorner, SppPoint pOtherCorner, int fLineEnd, int xOrg, int yOrg, double xFtrans, double yFtrans)
#else
miRoundCap(pGC, pCenter, pEnd, pCorner, pOtherCorner, fLineEnd, xOrg, yOrg, xFtrans, yFtrans)
     miGC *pGC;
     SppPoint pCenter, pEnd, pCorner, pOtherCorner;
     int fLineEnd;
     int xOrg, yOrg;
     double xFtrans, yFtrans;
#endif
{
  int		cpt;
  double	width;
  SppArc	arc;
  SppPoint	*pArcPts;

  width = (pGC->lineWidth ? pGC->lineWidth : 1);

  arc.x = pCenter.x - width/2;
  arc.y = pCenter.y - width/2;
  arc.width = width;
  arc.height = width;
  arc.angle1 = -miDatan2 (pCorner.y - pCenter.y, pCorner.x - pCenter.x);
  if(PTISEQUAL(pCenter, pEnd))
    arc.angle2 = - 180.0;
  else {
	 arc.angle2 = -miDatan2 (pOtherCorner.y - pCenter.y, pOtherCorner.x - pCenter.x) - arc.angle1;
	 if (arc.angle2 < 0)
	   arc.angle2 += 360.0;
       }

  /* convert semicircle to a polyline, and fill */
  pArcPts = (SppPoint *)NULL;
  if ((cpt = miGetArcPts (&arc, 0, &pArcPts)))
    /* by drawing with miFillSppPoly and setting the endpoints of the arc
     * to be the corners, we assure that the cap will meet up with the
     * rest of the line */
    miFillSppPoly (pGC, cpt, pArcPts, -xOrg, -yOrg, xFtrans, yFtrans);
  free (pArcPts);
}

/*
 * To avoid inaccuracy at the cardinal points, use trig functions
 * which are exact for those angles
 */

#define Dsin(d)	((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
#define Dcos(d)	((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))
#define mod(a,b)	((a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

static double
#ifdef _HAVE_PROTOS
miDcos (double a)
#else
miDcos (a)
     double a;
#endif
{
  int	i;

  if (floor (a/90) == a/90) 
    {
      i = (int) (a/90.0);
      switch (mod (i, 4)) 
	{
	case 0: return 1;
	case 1: return 0;
	case 2: return -1;
	case 3: return 0;
	}
    }
  return cos (a * M_PI / 180.0);
}

static double
#ifdef _HAVE_PROTOS
miDsin (double a)
#else
miDsin (a)
     double a;
#endif
{
  int	i;

  if (floor (a/90) == a/90) 
    {
      i = (int) (a/90.0);
      switch (mod (i, 4)) 
	{
	case 0: return 0;
	case 1: return 1;
	case 2: return 0;
	case 3: return -1;
	}
    }
  return sin (a * M_PI / 180.0);
}

static double
#ifdef _HAVE_PROTOS
miDasin (double v)
#else
miDasin (v)
     double v;
#endif
{
  if (v == 0)
    return 0.0;
  if (v == 1.0)
    return 90.0;
  if (v == -1.0)
    return -90.0;
  return asin(v) * (180.0 / M_PI);
}

static double 
#ifdef _HAVE_PROTOS
miDatan2 (double dy, double dx)
#else
miDatan2 (dy, dx)
     double dy, dx;
#endif
{
  if (dy == 0) 
    {
      if (dx >= 0)
	return 0.0;
      return 180.0;
    } 
  else if (dx == 0) 
    {
      if (dy > 0)
	return 90.0;
      return -90.0;
    } 
  else if (FABS(dy) == FABS(dx)) 
    {
      if (dy > 0) 
	{
	  if (dx > 0)
	    return 45.0;
	  return 135.0;
	} 
      else 
	{
	  if (dx > 0)
	    return 315.0;
	  return 225.0;
	}
    } 
  else 
    return atan2 (dy, dx) * (180.0 / M_PI);
}

/* MIGETARCPTS -- Converts an arc into a set of line segments, so the
 * resulting polygon can be filled -- a helper routine for filled arc and
 * line (round cap) code.  Returns the number of points in the arc.  Note
 * that it takes a pointer to a pointer to where it should put the points
 * and an index (cpt).  This procedure allocates the space necessary to fit
 * the arc points.  Sometimes it's convenient for those points to be at the
 * end of an existing array. (For example, if we want to leave a spare
 * point to make sectors instead of segments.)  So we pass in the
 * mi_xmalloc()ed chunk that contains the array and an index saying where
 * we should start stashing the points.  If there isn't an array already,
 * we just pass in a null pointer and count on mi_xrealloc() to handle the
 * null pointer correctly.
 */
static int
#ifdef _HAVE_PROTOS
miGetArcPts (const SppArc *parc, int cpt, SppPoint **ppPts)
#else
miGetArcPts (parc, cpt, ppPts)
     const SppArc *parc;
     int cpt;
     SppPoint **ppPts;
#endif
/* parc points to an arc */
/* cpt = number of points already in arc list */
/* ppPts is pointer to pointer to arc-list -- modified */
{
  double st;			/* Start Theta, start angle */
  double et;			/* End Theta, offset from start theta */
  double dt;			/* Delta Theta, angle to sweep ellipse */
  double cdt;			/* Cos Delta Theta, actually 2 cos(dt) */
  double x0, y0;		/* recurrence formula needs 2 points to start*/
  double x1, y1;
  double x2, y2;		/* this will be the new point generated */
  double xc, yc;		/* the center point */
  int count, i;
  SppPoint *poly;
  miIntPoint last;		/* last point on integer boundaries */

  /* The spec says that positive angles indicate counterclockwise motion.
     Given our coordinate system (with 0,0 in the upper left corner), the
     screen appears flipped in Y.  The easiest fix is to negate the angles
     given. */
  st = - parc->angle1;
  et = - parc->angle2;

  /* Try to get a delta theta that is within 1/2 pixel.  Then adjust it
   * so that it divides evenly into the total.
   * I'm just using cdt 'cause I'm lazy.
   */
  cdt = parc->width;
  if (parc->height > cdt)
    cdt = parc->height;
  cdt *= 0.5;
  if (cdt <= 0)
    return 0;
  if (cdt < 1.0)
    cdt = 1.0;
  dt = miDasin (1.0 / cdt);	/* minimum step necessary */
  count = (int)(et/dt);
  count = abs(count) + 1;
  dt = et/count;	
  count++;

  cdt = 2 * miDcos(dt);
  poly = (SppPoint *) mi_xrealloc(*ppPts,
				     (cpt + count) * sizeof(SppPoint));
  if (poly == NULL)
    return 0;
  *ppPts = poly;

  xc = 0.5 * parc->width;		/* store half width and half height */
  yc = 0.5 * parc->height;
    
  x0 = xc * miDcos(st);
  y0 = yc * miDsin(st);
  x1 = xc * miDcos(st + dt);
  y1 = yc * miDsin(st + dt);
  xc += parc->x;		/* by adding initial point, these become */
  yc += parc->y;		/* the center point */

  poly[cpt].x = (xc + x0);
  poly[cpt].y = (yc + y0);
  poly[cpt + 1].x = (xc + x1);
  poly[cpt + 1].y = (yc + y1);
  last.x = IROUND(xc + x1);
  last.y = IROUND(yc + y1);

  for (i = 2; i < count; i++)
    {
      x2 = cdt * x1 - x0;
      y2 = cdt * y1 - y0;

      poly[cpt + i].x = (xc + x2);
      poly[cpt + i].y = (yc + y2);

      x0 = x1; y0 = y1;
      x1 = x2; y1 = y2;
    }
  /* adjust the last point */
  if (FABS(parc->angle2) >= 360.0)
    poly[cpt +i -1] = poly[0];
  else
    {
      poly[cpt +i -1].x = (miDcos(st + et) * 0.5 * parc->width + xc);
      poly[cpt +i -1].y = (miDsin(st + et) * 0.5 * parc->height + yc);
    }

  return count;
}

#define ADD_REALLOC_STEP	20

/* helper function called by miComputeArcs() above */
static void
#ifdef _HAVE_PROTOS
addCap (miArcCapStruct **capsp, int *ncapsp, int *sizep, int end, int arcIndex)
#else
addCap (capsp, ncapsp, sizep, end, arcIndex)
     miArcCapStruct **capsp;
     int *ncapsp;
     int *sizep;
     int end;
     int arcIndex;
#endif
{
  int newsize;
  miArcCapStruct *cap;

  if (*ncapsp == *sizep)
    {
      newsize = *sizep + ADD_REALLOC_STEP;
      cap = (miArcCapStruct *) mi_xrealloc (*capsp,
					 newsize * sizeof (**capsp));
      *sizep = newsize;
      *capsp = cap;
    }
  cap = &(*capsp)[*ncapsp];
  cap->end = end;
  cap->arcIndex = arcIndex;
  ++*ncapsp;
}

/* helper function called by miComputeArcs() above */
static void
#ifdef _HAVE_PROTOS
addJoin (miArcJoinStruct **joinsp, int *njoinsp, int *sizep, int end0, int index0, int phase0, int end1, int index1, int phase1)
#else
addJoin (joinsp, njoinsp, sizep, end0, index0, phase0, end1, index1, phase1)
     miArcJoinStruct **joinsp;
     int *njoinsp, *sizep;
     int end0, index0, phase0;
     int end1, index1, phase1;
#endif
{
  int newsize;
  miArcJoinStruct *join;

  if (*njoinsp == *sizep)
    {
      newsize = *sizep + ADD_REALLOC_STEP;
      join = (miArcJoinStruct *) mi_xrealloc (*joinsp,
					   newsize * sizeof (**joinsp));
      *sizep = newsize;
      *joinsp = join;
    }
  join = &(*joinsp)[*njoinsp];
  join->end0 = end0;
  join->arcIndex0 = index0;
  join->phase0 = phase0;
  join->end1 = end1;
  join->arcIndex1 = index1;
  join->phase1 = phase1;
  ++*njoinsp;
}

/* helper function called by miComputeArcs() above */
static miArcData *
#ifdef _HAVE_PROTOS
addArc (miArcData **arcsp, int *narcsp, int *sizep, const miArc *xarc)
#else
addArc (arcsp, narcsp, sizep, xarc)
     miArcData **arcsp;
     int *narcsp;
     int *sizep;
     const miArc *xarc;
#endif
{
  int newsize;
  miArcData *new_arc;

  if (*narcsp == *sizep)
    {
      newsize = *sizep + ADD_REALLOC_STEP;
      new_arc = (miArcData *) mi_xrealloc (*arcsp,
					   newsize * sizeof (**arcsp));
      *sizep = newsize;
      *arcsp = new_arc;
    }
  new_arc = &(*arcsp)[*narcsp];
  new_arc->arc = *xarc;
  ++*narcsp;
  return new_arc;
}


/* A sub-module that computes arc lengths via a polygonal approximation to
 * the arc.  External functions are computeDashMap(), which should be
 * called first, and the primary function computeAngleFromPath().
 * They are called by miComputeArcs() above.
 */

#define dashIndexToAngle(di)	((((double) (di)) * 90.0) / ((double) DASH_MAP_SIZE - 1))
#define xAngleToDashIndex(xa)	((((long) (xa)) * (DASH_MAP_SIZE - 1)) / (90 * 64))
#define dashIndexToXAngle(di)	((((long) (di)) * (90 * 64)) / (DASH_MAP_SIZE - 1))
#define dashXAngleStep	(((double) (90 * 64)) / ((double) (DASH_MAP_SIZE - 1)))

/* forward references (functions in this sub-module) */
static double angleToLength ____P((int angle, const dashMap *map));
static int lengthToAngle ____P((double len, const dashMap *map));

static void
#ifdef _HAVE_PROTOS
computeDashMap (const miArc *arcp, dashMap *map)
#else
computeDashMap (arcp, map)
     const miArc *arcp;
     dashMap *map;
#endif
{
  int di;
  double a, x, y, prevx = 0.0, prevy = 0.0, dist;

  for (di = 0; di < DASH_MAP_SIZE; di++) 
    {
      a = dashIndexToAngle (di);
      x = (double)(0.5 * arcp->width) * miDcos (a);
      y = (double)(0.5 * arcp->height) * miDsin (a);
      if (di == 0) 
	map->map[di] = 0.0;
      else 
	{
	  dist = hypot (x - prevx, y - prevy);
	  map->map[di] = map->map[di - 1] + dist;
	}
      prevx = x;
      prevy = y;
    }
}

static double
#ifdef _HAVE_PROTOS
angleToLength (int angle, const dashMap *map)
#else
angleToLength (angle, map)
     int angle;
     const dashMap *map;
#endif
{
  double len, excesslen, sidelen = map->map[DASH_MAP_SIZE - 1], totallen;
  int    di;
  int	 excess;
  bool	 oddSide = false;

  totallen = 0;
  if (angle >= 0) 
    {
      while (angle >= 90 * 64) 
	{
	  angle -= 90 * 64;
	  totallen += sidelen;
	  oddSide = (oddSide ? false : true);
	}
    } 
  else 
    {
      while (angle < 0) 
	{
	  angle += 90 * 64;
	  totallen -= sidelen;
	  oddSide = (oddSide ? false : true);
	}
    }
  if (oddSide)
    angle = 90 * 64 - angle;
		
  di = xAngleToDashIndex (angle);
  excess = angle - dashIndexToXAngle (di);

  len = map->map[di];
  /*
   * linearly interpolate between this point and the next
   */
  if (excess > 0) 
    {
      excesslen = (map->map[di + 1] - map->map[di]) *
	((double) excess) / dashXAngleStep;
      len += excesslen;
    }
  if (oddSide)
    totallen += (sidelen - len);
  else
    totallen += len;
  return totallen;
}

/*
 * len is along the arc, but may be more than one rotation
 */

static int
#ifdef _HAVE_PROTOS
lengthToAngle (double len, const dashMap *map)
#else
lengthToAngle (len, map)
     double len;
     const dashMap *map;
#endif
{
  double sidelen = map->map[DASH_MAP_SIZE - 1];
  int angle, angleexcess;
  bool oddSide = false;
  int a0, a1, a;

  angle = 0;
  /*
   * step around the ellipse, subtracting sidelens and
   * adding 90 degrees.  oddSide will tell if the
   * map should be interpolated in reverse
   */
  if (len >= 0) 
    {
      if (sidelen == 0)
	return 2 * FULLCIRCLE;	/* infinity */
      while (len >= sidelen) 
	{
	  angle += 90 * 64;
	  len -= sidelen;
	  oddSide = (oddSide ? false : true);
	}
    } 
  else 
    {
      if (sidelen == 0)
	return -2 * FULLCIRCLE;	/* infinity */
      while (len < 0) 
	{
	  angle -= 90 * 64;
	  len += sidelen;
	  oddSide = (oddSide ? false : true);
	}
    }
  if (oddSide)
    len = sidelen - len;
  a0 = 0;
  a1 = DASH_MAP_SIZE - 1;
  /*
   * binary search for the closest pre-computed length
   */
  while (a1 - a0 > 1) 
    {
      a = (a0 + a1) / 2;
      if (len > map->map[a])
	a0 = a;
      else
	a1 = a;
    }
  angleexcess = dashIndexToXAngle (a0);
  /*
   * linearly interpolate to the next point
   */
  angleexcess += (int)((len - map->map[a0]) /
		       (map->map[a0+1] - map->map[a0]) * dashXAngleStep);
  if (oddSide)
    angle += (90 * 64) - angleexcess;
  else
    angle += angleexcess;
  return angle;
}

/* Compute the angle of an ellipse which corresponds to the given path
 * length.  The correct solution to this problem is an elliptic integral,
 * so we'll punt and approximate (it's only for dashes anyway...).  This
 * approximation uses a polygon.
 *
 * The remaining portion of len is stored in *lenp.  This will be negative
 * if the arc extends beyond len and positive if len extends beyond the
 * arc.
 */

static int
#ifdef _HAVE_PROTOS
computeAngleFromPath (int startAngle, int endAngle, const dashMap *map, int *lenp, bool backwards)
/* start, endAngle are normalized angles in 64ths of a degree */
#else
computeAngleFromPath (startAngle, endAngle, map, lenp, backwards)
     int startAngle, endAngle;
     const dashMap *map;
     int *lenp;
     bool backwards;
#endif
{
  int	a0, a1, a;
  double len0;
  int	len;

  a0 = startAngle;
  a1 = endAngle;
  len = *lenp;
  if (backwards) 
    /* flip the problem around to be forwards */
    {
      a0 = FULLCIRCLE - a0;
      a1 = FULLCIRCLE - a1;
    }

  if (a1 < a0)
    a1 += FULLCIRCLE;
  len0 = angleToLength (a0, map);
  a = lengthToAngle (len0 + len, map);
  if (a > a1) 
    {
      a = a1;
      len = (int)(len - angleToLength (a1, map) - len0);
    } 
  else
    len = 0;
  if (backwards)
    a = FULLCIRCLE - a;

  *lenp = len;
  return a;
}


/* Draw a degenerate (zero width/height) arc.  Left and right faces must be
 * supplied.  This calls miPolyFillRect() to do the drawing.
 */

static void
#ifdef _HAVE_PROTOS
drawZeroArc (miGC *pGC, const miArc *tarc, unsigned int lw, miArcFace *left, miArcFace *right)
#else
drawZeroArc (pGC, tarc, lw, left, right)
     miGC *pGC;
     const miArc *tarc;
     unsigned int lw;
     miArcFace *left, *right;	/* these save arc endpoints */
#endif
{
  double	x0 = 0, y0 = 0, x1 = 0, y1 = 0, w, h, x, y;
  double	xmax, ymax, xmin, ymin;
  int		a0, a1;
  double	a, startAngle, endAngle;
  double	l, lx, ly;

  l = 0.5 * lw;
  a0 = tarc->angle1;
  a1 = tarc->angle2;
  if (a1 > FULLCIRCLE)
    a1 = FULLCIRCLE;
  else if (a1 < -FULLCIRCLE)
    a1 = -FULLCIRCLE;
  w = 0.5 * tarc->width;
  h = 0.5 * tarc->height;
  /*
   * play in X coordinates right away
   */
  startAngle = - ((double) a0 / 64.0);
  endAngle = - ((double) (a0 + a1) / 64.0);
	
  xmax = -w;
  xmin = w;
  ymax = -h;
  ymin = h;
  a = startAngle;
  for (;;)
    {
      x = w * miDcos(a);
      y = h * miDsin(a);
      if (a == startAngle)
	{
	  x0 = x;
	  y0 = y;
	}
      if (a == endAngle)
	{
	  x1 = x;
	  y1 = y;
	}
      if (x > xmax)
	xmax = x;
      if (x < xmin)
	xmin = x;
      if (y > ymax)
	ymax = y;
      if (y < ymin)
	ymin = y;
      if (a == endAngle)
	break;
      if (a1 < 0)		/* clockwise */
	{
	  if (floor (a / 90.0) == floor (endAngle / 90.0))
	    a = endAngle;
	  else
	    a = 90 * (floor (a/90.0) + 1);
	}
      else
	{
	  if (ceil (a / 90.0) == ceil (endAngle / 90.0))
	    a = endAngle;
	  else
	    a = 90 * (ceil (a/90.0) - 1);
	}
    }
  lx = ly = l;
  if ((x1 - x0) + (y1 - y0) < 0)
    lx = ly = -l;
  if (h)
    ly = 0.0;
  else
    lx = 0.0;
  if (right)
    {
      right->center.x = x0;
      right->center.y = y0;
      right->clock.x = x0 - lx;
      right->clock.y = y0 - ly;
      right->counterClock.x = x0 + lx;
      right->counterClock.y = y0 + ly;
    }
  if (left)
    {
      left->center.x = x1;
      left->center.y = y1;
      left->clock.x = x1 + lx;
      left->clock.y = y1 + ly;
      left->counterClock.x = x1 - lx;
      left->counterClock.y = y1 - ly;
    }
	
  x0 = xmin;
  x1 = xmax;
  y0 = ymin;
  y1 = ymax;
  if (ymin != y1) 
    {
      xmin = -l;
      xmax = l;
    } 
  else 
    {
      ymin = -l;
      ymax = l;
    }
  if (xmax != xmin && ymax != ymin) 
    {
      int	minx, maxx, miny, maxy;
      miRectangle  rect;

      minx = ICEIL(xmin + w) + tarc->x;
      maxx = ICEIL(xmax + w) + tarc->x;
      miny = ICEIL(ymin + h) + tarc->y;
      maxy = ICEIL(ymax + h) + tarc->y;
      rect.x = minx;
      rect.y = miny;
      rect.width = maxx - minx;
      rect.height = maxy - miny;
      miPolyFillRect (pGC, 1, &rect);
    }
}

/***********************************************************************/

/* The following sub-module scan-converts a nondegenerate (nonzero width,
 * nonzero height) wide arc.  It provides two functions: drawArc() and
 * fillSpans().
 */

/* This code is not good.  It uses global variables (see below). */

/***********************************************************************/

struct bound 
{
  double min, max;
};

struct ibound 
{
  int min, max;
};

/*
 * these are all y value bounds
 */
struct arc_bound 
{
  struct bound ellipse;
  struct bound inner, outer;
  struct bound right, left;
  struct ibound inneri, outeri;
};

struct line 
{
  double m, b;			/* for y = mx + b */
  bool valid;
};

/* quantities frequently used when drawn an ellipse or elliptic arc */
struct accelerators 
{
  double		tail_y;	/* "y value associated with bottom of tail" */
  double		h2;	/* half-height squared */
  double		w2;	/* half-width squared */
  double		h4;	/* half-height raised to 4th power */
  double		w4;	/* half-width raised to 4th power */
  double		h2mw2;	/* h2 minus w2 */
  double		h2l;	/* h2 times l (i.e. half the line width) */
  double		w2l;	/* w2 times l (i.e. half the line width) */
  double		fromIntX; /* 0.5 if width is odd, otherwise 0.0 */
  double		fromIntY; /* 0.5 if height is oddd, otherwise 0.0 */
  struct line	left, right;
  int		yorgu;
  int		yorgl;
  int		xorg;
};

struct arc_def 
{
  double	w, h;		/* half-width, half-height */
  double	l;		/* half of line width */
  double	a0, a1;		/* start angle, and angle range */
};

#define boundedLe(value, bounds)\
	((bounds).min <= (value) && (value) <= (bounds).max)

#define intersectLine(y,line) (line.m * (y) + line.b)


/* forward references */
static double hookEllipseY ____P((double scan_y, const struct arc_bound *bound, const struct accelerators *acc, bool left));
static double hookX ____P((double scan_y, const struct arc_def *def, const struct arc_bound *bound, const struct accelerators *acc, bool left));
static double innerXfromXY ____P((double x, double y, const struct accelerators *acc));
static double innerYfromXY ____P((double x, double y, const struct accelerators *acc));
static double innerYfromY ____P((double y, const struct arc_def *def, const struct accelerators *acc));
static double outerXfromXY ____P((double x, double y, const struct accelerators *acc));
static double outerYfromXY ____P((double x, double y, const struct accelerators *acc));
static double tailX ____P((double K, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc));
static void arcSpan ____P((int y, int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, int mask));
static void arcSpan0 ____P((int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, int mask));
static void computeAcc ____P((const miArc *tarc, unsigned int lw, struct arc_def *def, struct accelerators *acc));
static void computeBound ____P((const struct arc_def *def, struct arc_bound *bound, struct accelerators *acc, miArcFace *right, miArcFace *left));
static void computeLine ____P((double x1, double y1, double x2, double y2, struct line *line));
static void newFinalSpan ____P((int y, int xmin, int xmax));
static void tailEllipseY ____P((const struct arc_def *def, struct accelerators *acc));
static void tailSpan ____P((int y, int lw, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, int mask));


static double
#ifdef _HAVE_PROTOS
tailX (double K, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc)
#else
tailX (K, def, bounds, acc)
     double K;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
#endif
{
  double w, h, r;
  double Hs, Hf, WH, Vk, Nk, Fk, Vr, N, Nc, Z, rs;
  double A, T, b, d, x, y, t, hepp, hepm;
  int flip, solution;
  double xs[2];
  double *xp;
  
  w = def->w;
  h = def->h;
  r = def->l;
  rs = r * r;
  Hs = acc->h2;
  WH = -acc->h2mw2;
  Nk = def->w * r;
  Vk = (Nk * Hs) / (WH + WH);
  Hf = acc->h4;
  Nk = (Hf - Nk * Nk) / WH;
  if (K == 0.0) 
    {
      if (Nk < 0.0 && -Nk < Hs) 
	{
	  xs[0] = w * sqrt(1 + Nk / Hs) - sqrt(rs + Nk);
	  xs[1] = w - r;
	  if (acc->left.valid && boundedLe(K, bounds->left) &&
	      !boundedLe(K, bounds->outer) && xs[0] >= 0.0 && xs[1] >= 0.0)
	    return xs[1];
	  if (acc->right.valid && boundedLe(K, bounds->right) &&
	      !boundedLe(K, bounds->inner) && xs[0] <= 0.0 && xs[1] <= 0.0)
	    return xs[1];
	  return xs[0];
	}
      return w - r;
    }
  Fk = Hf / WH;
  hepp = h + EPSILON;
  hepm = h - EPSILON;
  N = (K * K + Nk) / 6.0;
  Nc = N * N * N;
  Vr = Vk * K;
  xp = xs;
  xs[0] = 0.0;
  t = Nc + Vr * Vr;
  d = Nc + t;
  if (d < 0.0) 
    {
      d = Nc;
      b = N;
      if ( (b < 0.0) == (t < 0.0) )
	{
	  b = -b;
	  d = -d;
	}
      Z = N - 2.0 * b * cos(acos(-t / d) / 3.0);
      if ( (Z < 0.0) == (Vr < 0.0) )
	flip = 2;
      else
	flip = 1;
    }
  else
    {
      d = Vr * sqrt(d);
      Z = N + cbrt(t + d) + cbrt(t - d);
      flip = 0;
    }
  A = sqrt((Z + Z) - Nk);
  T = (Fk - Z) * K / A;
  solution = false;
  b = -A + K;
  d = b * b - 4 * (Z + T);
  if (d >= 0 && flip == 2)
    {
      d = sqrt(d);
      y = (b + d) / 2;
      if ((y >= 0.0) && (y < hepp))
	{
	  solution = true;
	  if (y > hepm)
	    y = h;
	  t = y / h;
	  x = w * sqrt(1 - (t * t));
	  t = K - y;
	  t = sqrt(rs - (t * t));
	  *xp++ = x - t;
	}
    }
  b = A + K;
  d = b * b - 4 * (Z - T);
  /* Because of the large magnitudes involved, we lose enough precision
   * that sometimes we end up with a negative value near the axis, when
   * it should be positive.  This is a workaround.
   */
  if (d < 0 && !solution)
    d = 0.0;
  if (d >= 0) 
    {
      d = sqrt(d);
      y = (b + d) / 2;
      if (y < hepp)
	{
	  if (y > hepm)
	    y = h;
	  t = y / h;
	  x = w * sqrt(1 - (t * t));
	  t = K - y;
	  *xp++ = x - sqrt(rs - (t * t));
	}
      y = (b - d) / 2;
      if (y >= 0.0 && flip == 1)
	{
	  if (y > hepm)
	    y = h;
	  t = y / h;
	  x = w * sqrt(1 - (t * t));
	  t = K - y;
	  t = sqrt(rs - (t * t));
	  *xp++ = x - t;
	}
    }
  if (xp > &xs[1]) 
    {
      if (acc->left.valid && boundedLe(K, bounds->left) &&
	  !boundedLe(K, bounds->outer) && xs[0] >= 0.0 && xs[1] >= 0.0)
	return xs[1];
      if (acc->right.valid && boundedLe(K, bounds->right) &&
	  !boundedLe(K, bounds->inner) && xs[0] <= 0.0 && xs[1] <= 0.0)
	return xs[1];
    }
  return xs[0];
}

/*
 * This computes the ellipse y value associated with the
 * bottom of the tail.
 */
static void
#ifdef _HAVE_PROTOS
tailEllipseY (const struct arc_def *def, struct accelerators *acc)
#else
tailEllipseY (def, acc)
     const struct arc_def *def;
     struct accelerators *acc;
#endif
{
  double t;

  acc->tail_y = 0.0;
  if (def->w == def->h)
    return;
  t = def->l * def->w;
  if (def->w > def->h) 
    {
      if (t < acc->h2)
	return;
    } 
  else 
    {
      if (t > acc->h2)
	return;
    }
  t = 2.0 * def->h * t;
  t = (CUBE_ROOT_4 * acc->h2 - cbrt(t * t)) / acc->h2mw2;
  if (t > 0.0)
    acc->tail_y = def->h / CUBE_ROOT_2 * sqrt(t);
}

/*
 * inverse functions -- compute edge coordinates
 * from the ellipse (actually, from its precomputed accelerators)
 */

static double
#ifdef _HAVE_PROTOS
outerXfromXY (double x, double y, const struct accelerators *acc)
#else
outerXfromXY (x, y, def, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return x + (x * acc->h2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
outerYfromXY (double x, double y, const struct accelerators *acc)
#else
outerYfromXY (x, y, def, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return y + (y * acc->w2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
innerXfromXY (double x, double y, const struct accelerators *acc)
#else
innerXfromXY (x, y, def, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return x - (x * acc->h2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
innerYfromXY (double x, double y, const struct accelerators *acc)
#else
innerYfromXY (x, y, def, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return y - (y * acc->w2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
innerYfromY (double y, const struct arc_def *def, const struct accelerators *acc)
#else
innerYfromY (y, def, acc)
     double y;
     const struct arc_def *def;
     const struct accelerators *acc;
#endif
{
  double x;
										 x = (def->w / def->h) * sqrt (acc->h2 - y*y);
										 return y - (y * acc->w2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
									       }
     
/* compute a line through two points */
static void
#ifdef _HAVE_PROTOS
computeLine (double x1, double y1, double x2, double y2, struct line *line)
#else
computeLine (x1, y1, x2, y2, line)
     double x1, y1, x2, y2;
     struct line *line;
#endif
{
  if (y1 == y2)
    line->valid = false;
  else 
    {
      line->m = (x1 - x2) / (y1 - y2);
      line->b = x1  - y1 * line->m;
      line->valid = true;
    }
}

/* Compute various accelerators for an ellipse.  These are simply values
   that are used repeatedly in the computations. */
static void
#ifdef _HAVE_PROTOS
computeAcc (const miArc *tarc, unsigned int lw, struct arc_def *def, struct accelerators *acc)
#else
computeAcc (tarc, lw, def, acc)
     const miArc *tarc;
     unsigned int lw;
     struct arc_def *def;
     struct accelerators *acc;
#endif
{
  def->w = 0.5 * (double)tarc->width;
  def->h = 0.5 * (double)tarc->height;
  def->l = 0.5 * (double)lw;
  acc->h2 = def->h * def->h;
  acc->w2 = def->w * def->w;
  acc->h4 = acc->h2 * acc->h2;
  acc->w4 = acc->w2 * acc->w2;
  acc->h2l = acc->h2 * def->l;
  acc->w2l = acc->w2 * def->l;
  acc->h2mw2 = acc->h2 - acc->w2;
  acc->fromIntX = (tarc->width & 1) ? 0.5 : 0.0;
  acc->fromIntY = (tarc->height & 1) ? 0.5 : 0.0;
  acc->xorg = tarc->x + (int)(tarc->width >> 1);
  acc->yorgu = tarc->y + (int)(tarc->height >> 1);
  acc->yorgl = acc->yorgu + (tarc->height & 1);
  tailEllipseY (def, acc);
}
		
/* Compute y value bounds of various portions of the arc, the outer edge,
   the ellipse and the inner edge. */
static void
#ifdef _HAVE_PROTOS
computeBound (const struct arc_def *def, struct arc_bound *bound, struct accelerators *acc, miArcFace *right, miArcFace *left)
#else
computeBound (def, bound, acc, right, left)
     const struct arc_def *def;
     struct arc_bound *bound;
     struct accelerators *acc;
     miArcFace *right, *left;
#endif
{
  double		t;
  double		innerTaily;
  double		tail_y;
  struct bound	innerx, outerx;
  struct bound	ellipsex;

  bound->ellipse.min = Dsin (def->a0) * def->h;
  bound->ellipse.max = Dsin (def->a1) * def->h;
  if (def->a0 == 45 && def->w == def->h)
    ellipsex.min = bound->ellipse.min;
  else
    ellipsex.min = Dcos (def->a0) * def->w;
  if (def->a1 == 45 && def->w == def->h)
    ellipsex.max = bound->ellipse.max;
  else
    ellipsex.max = Dcos (def->a1) * def->w;
  bound->outer.min = outerYfromXY (ellipsex.min, bound->ellipse.min, acc);
  bound->outer.max = outerYfromXY (ellipsex.max, bound->ellipse.max, acc);
  bound->inner.min = innerYfromXY (ellipsex.min, bound->ellipse.min, acc);
  bound->inner.max = innerYfromXY (ellipsex.max, bound->ellipse.max, acc);

  outerx.min = outerXfromXY (ellipsex.min, bound->ellipse.min, acc);
  outerx.max = outerXfromXY (ellipsex.max, bound->ellipse.max, acc);
  innerx.min = innerXfromXY (ellipsex.min, bound->ellipse.min, acc);
  innerx.max = innerXfromXY (ellipsex.max, bound->ellipse.max, acc);
	
  /* Save the line end points for the cap code to use.  Careful here, these
   * are in Cartesian coordinates (y increasing upwards) while the cap code
   * uses inverted coordinates (y increasing downwards).
   */

  if (right) 
    {
      right->counterClock.y = bound->outer.min;
      right->counterClock.x = outerx.min;
      right->center.y = bound->ellipse.min;
      right->center.x = ellipsex.min;
      right->clock.y = bound->inner.min;
      right->clock.x = innerx.min;
    }

  if (left) 
    {
      left->clock.y = bound->outer.max;
      left->clock.x = outerx.max;
      left->center.y = bound->ellipse.max;
      left->center.x = ellipsex.max;
      left->counterClock.y = bound->inner.max;
      left->counterClock.x = innerx.max;
    }

  bound->left.min = bound->inner.max;
  bound->left.max = bound->outer.max;
  bound->right.min = bound->inner.min;
  bound->right.max = bound->outer.min;

  computeLine (innerx.min, bound->inner.min, outerx.min, bound->outer.min,
	       &acc->right);
  computeLine (innerx.max, bound->inner.max, outerx.max, bound->outer.max,
	       &acc->left);

  if (bound->inner.min > bound->inner.max) 
    {
      t = bound->inner.min;
      bound->inner.min = bound->inner.max;
      bound->inner.max = t;
    }
  tail_y = acc->tail_y;
  if (tail_y > bound->ellipse.max)
    tail_y = bound->ellipse.max;
  else if (tail_y < bound->ellipse.min)
    tail_y = bound->ellipse.min;
  innerTaily = innerYfromY (tail_y, def, acc);
  if (bound->inner.min > innerTaily)
    bound->inner.min = innerTaily;
  if (bound->inner.max < innerTaily)
    bound->inner.max = innerTaily;
  bound->inneri.min = ICEIL(bound->inner.min - acc->fromIntY);
  bound->inneri.max = IFLOOR(bound->inner.max - acc->fromIntY);
  bound->outeri.min = ICEIL(bound->outer.min - acc->fromIntY);
  bound->outeri.max = IFLOOR(bound->outer.max - acc->fromIntY);
}

/*
 * this section computes the x value of the span at y 
 * intersected with the specified face of the ellipse.
 *
 * this is the min/max X value over the set of normal
 * lines to the entire ellipse,  the equation of the
 * normal lines is:
 *
 *     ellipse_x h^2                   h^2
 * x = ------------ y + ellipse_x (1 - --- )
 *     ellipse_y w^2                   w^2
 *
 * compute the derivative with-respect-to ellipse_y and solve
 * for zero:
 *    
 *       (w^2 - h^2) ellipse_y^3 + h^4 y
 * 0 = - ----------------------------------
 *       h w ellipse_y^2 sqrt (h^2 - ellipse_y^2)
 *
 *             (   h^4 y     )
 * ellipse_y = ( ----------  ) ^ (1/3)
 *             ( (h^2 - w^2) )
 *
 * The other two solutions to the equation are imaginary.
 *
 * This gives the position on the ellipse which generates
 * the normal with the largest/smallest x intersection point.
 *
 * Now compute the second derivative to check whether
 * the intersection is a minimum or maximum:
 *
 *    h (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 * -  -------------------------------------------
 *          w y0^3 (sqrt (h^2 - y^2)) ^ 3
 *
 * as we only care about the sign,
 *
 * - (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 *
 * or (to use accelerators),
 *
 * y0^3 (h^2 - w^2) - h^2 y (3y0^2 - 2h^2) 
 *
 */

/* Compute the position on the ellipse whose normal line intersects the
   given scan line maximally. */
static double
#ifdef _HAVE_PROTOS
hookEllipseY (double scan_y, const struct arc_bound *bound, const struct accelerators *acc, bool left)
#else
hookEllipseY (scan_y, bound, acc, left)
     double scan_y;
     const struct arc_bound *bound;
     const struct accelerators *acc;
     bool left;
#endif
{
  double ret;

  if (acc->h2mw2 == 0) 
    {
      if ( (scan_y > 0 && (left ? false : true)) || (scan_y < 0 && left) )
	return bound->ellipse.min;
      return bound->ellipse.max;
    }
  ret = (acc->h4 * scan_y) / (acc->h2mw2);
  if (ret >= 0)
    return cbrt (ret);
  else
    return -cbrt (-ret);
}

/* Compute the X value of the intersection of the given scan line with the
   right side of the lower hook. */
static double
#ifdef _HAVE_PROTOS
hookX (double scan_y, const struct arc_def *def, const struct arc_bound *bound, const struct accelerators *acc, bool left)
#else
hookX (scan_y, def, bound, acc, left)
     double scan_y;
     const struct arc_def *def;
     const struct arc_bound *bound;
     const struct accelerators *acc;
     bool left;
#endif
{
  double	ellipse_y, x;
  double	maxMin;

  if (def->w != def->h) 
    {
      ellipse_y = hookEllipseY (scan_y, bound, acc, left);
      if (boundedLe (ellipse_y, bound->ellipse)) 
	{
	  /*
	   * compute the value of the second
	   * derivative
	   */
	  maxMin = ellipse_y*ellipse_y*ellipse_y * acc->h2mw2 -
	    acc->h2 * scan_y * (3 * ellipse_y*ellipse_y - 2*acc->h2);
	  if ((left && maxMin > 0) || ((left ? false : true) && maxMin < 0))
	    {
	      if (ellipse_y == 0)
		return def->w + left ? -def->l : def->l;
	      x = (acc->h2 * scan_y - ellipse_y * acc->h2mw2) *
		sqrt (acc->h2 - ellipse_y * ellipse_y) /
		  (def->h * def->w * ellipse_y);
	      return x;
	    }
	}
    }
  if (left) 
    {
      if (acc->left.valid && boundedLe (scan_y, bound->left)) 
	x = intersectLine (scan_y, acc->left);
      else 
	{
	  if (acc->right.valid)
	    x = intersectLine (scan_y, acc->right);
	  else
	    x = def->w - def->l;
	}
    } 
  else 
    {
      if (acc->right.valid && boundedLe (scan_y, bound->right)) 
	x = intersectLine (scan_y, acc->right);
      else 
	{
	  if (acc->left.valid)
	    x = intersectLine (scan_y, acc->left);
	  else
	    x = def->w - def->l;
	}
    }
  return x;
}

/* Generate the set of spans with the given y coordinate. */
static void
#ifdef _HAVE_PROTOS
arcSpan (int y, int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, int mask)
#else
arcSpan (y, lx, lw, rx, rw, def, bounds, acc, mask)
     int y, lx, lw, rx, rw;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
     int mask;
#endif
{
  int linx, loutx, rinx, routx;
  double x, altx;

  if (boundedLe (y, bounds->inneri)) 
    {
      linx = -(lx + lw);
      rinx = rx;
    } 
  else 
    {
      /*
       * intersection with left face
       */
      x = hookX (y + acc->fromIntY, def, bounds, acc, true);
      if (acc->right.valid
	  && boundedLe (y + acc->fromIntY, bounds->right))
	{
	  altx = intersectLine (y + acc->fromIntY, acc->right);
	  if (altx < x)
	    x = altx;
	}
      linx = -ICEIL(acc->fromIntX - x);
      rinx = ICEIL(acc->fromIntX + x);
    }

  if (boundedLe (y, bounds->outeri)) 
    {
      loutx = -lx;
      routx = rx + rw;
    } 
  else 
    {
      /*
       * intersection with right face
       */
      x = hookX (y + acc->fromIntY, def, bounds, acc, false);
      if (acc->left.valid
	  && boundedLe (y + acc->fromIntY, bounds->left))
	{
	  altx = x;
	  x = intersectLine (y + acc->fromIntY, acc->left);
	  if (x < altx)
	    x = altx;
	}
      loutx = -ICEIL(acc->fromIntX - x);
      routx = ICEIL(acc->fromIntX + x);
    }

  if (routx > rinx) 
    {
      if (mask & 1)
	newFinalSpan (acc->yorgu - y,
		      acc->xorg + rinx, acc->xorg + routx);
      if (mask & 8)
	newFinalSpan (acc->yorgl + y,
		      acc->xorg + rinx, acc->xorg + routx);
    }

  if (loutx > linx) 
    {
      if (mask & 2)
	newFinalSpan (acc->yorgu - y,
		      acc->xorg - loutx, acc->xorg - linx);
      if (mask & 4)
	newFinalSpan (acc->yorgl + y,
		      acc->xorg - loutx, acc->xorg - linx);
    }
}

static void
#ifdef _HAVE_PROTOS
arcSpan0 (int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, int mask)
#else
arcSpan0 (lx, lw, rx, rw, def, bounds, acc, mask)
     int lx, lw, rx, rw;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
     int mask;
#endif
{
  double x;

  if (boundedLe (0, bounds->inneri) 
      && acc->left.valid && boundedLe (0, bounds->left)
      && acc->left.b > 0)
    {
      x = def->w - def->l;
      if (acc->left.b < x)
	x = acc->left.b;
      lw = ICEIL(acc->fromIntX - x) - lx;
      rw += rx;
      rx = ICEIL(acc->fromIntX + x);
      rw -= rx;
    }
  arcSpan (0, lx, lw, rx, rw, def, bounds, acc, mask);
}

static void
#ifdef _HAVE_PROTOS
tailSpan (int y, int lw, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, int mask)
#else
tailSpan (y, lw, rw, def, bounds, acc, mask)
     int y, lw, rw;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
     int mask;
#endif
{
  double yy, xalt, x, lx, rx;
  int n;

  if (boundedLe(y, bounds->outeri))
    arcSpan (y, 0, lw, -rw, rw, def, bounds, acc, mask);
  else if (def->w != def->h) 
    {
      yy = y + acc->fromIntY;
      x = tailX(yy, def, bounds, acc);
      if (yy == 0.0 && x == -rw - acc->fromIntX)
	return;
      if (acc->right.valid && boundedLe (yy, bounds->right)) 
	{
	  rx = x;
	  lx = -x;
	  xalt = intersectLine (yy, acc->right);
	  if (xalt >= -rw - acc->fromIntX && xalt <= rx)
	    rx = xalt;
	  n = ICEIL(acc->fromIntX + lx);
	  if (lw > n) 
	    {
	      if (mask & 2)
		newFinalSpan (acc->yorgu - y,
			      acc->xorg + n, acc->xorg + lw);
	      if (mask & 4)
		newFinalSpan (acc->yorgl + y,
			      acc->xorg + n, acc->xorg + lw);
	    }
	  n = ICEIL(acc->fromIntX + rx);
	  if (n > -rw) 
	    {
	      if (mask & 1)
		newFinalSpan (acc->yorgu - y,
			      acc->xorg - rw, acc->xorg + n);
	      if (mask & 8)
		newFinalSpan (acc->yorgl + y,
			      acc->xorg - rw, acc->xorg + n);
	    }
	}
      arcSpan (y,
	       ICEIL(acc->fromIntX - x), 0,
	       ICEIL(acc->fromIntX + x), 0,
	       def, bounds, acc, mask);
    }
}


/* A sub-module that creates whole arcs out of pieces.  This code is very
   bad (as is clear from the use of global variables...). */

static struct finalSpan	**finalSpans = NULL;
static int		finalMiny = 0, finalMaxy = -1;
static int		finalSize = 0;
static int		nspans = 0;	/* total spans, not just y coords */

struct finalSpan 
{
  struct finalSpan	*next;
  int			min, max; /* x values */
};

static struct finalSpan    *freeFinalSpans, *tmpFinalSpan;

#define allocFinalSpan()   (freeFinalSpans ?\
			     ((tmpFinalSpan = freeFinalSpans), \
			      (freeFinalSpans = freeFinalSpans->next), \
				 (tmpFinalSpan->next = 0), \
				 tmpFinalSpan) : \
			     realAllocSpan ())

#define SPAN_CHUNK_SIZE    128

struct finalSpanChunk 
{
  struct finalSpan	data[SPAN_CHUNK_SIZE];
  struct finalSpanChunk	*next;
};

static struct finalSpanChunk	*chunks;

/* forward references */
static struct finalSpan * realAllocSpan ____P((void));
static struct finalSpan ** realFindSpan ____P((int y));
static void disposeFinalSpans ____P((void));
static void drawQuadrant ____P((struct arc_def *def, struct accelerators *acc, int a0, int a1, int mask, miArcFace *right, miArcFace *left, miArcSpanData *spdata));
static void mirrorSppPoint ____P((int quadrant, SppPoint *sppPoint));


static struct finalSpan *
#ifdef _HAVE_PROTOS
realAllocSpan (void)
#else
realAllocSpan ()
#endif
{
  struct finalSpanChunk	*newChunk;
  struct finalSpan	*span;
  int			i;

  newChunk = (struct finalSpanChunk *) mi_xmalloc (sizeof (struct finalSpanChunk));
  newChunk->next = chunks;
  chunks = newChunk;
  freeFinalSpans = span = newChunk->data + 1;
  for (i = 1; i < SPAN_CHUNK_SIZE-1; i++) 
    {
      span->next = span+1;
      span++;
    }
  span->next = (struct finalSpan *)NULL;
  span = newChunk->data;
  span->next = (struct finalSpan *)NULL;
  return span;
}

static void
#ifdef _HAVE_PROTOS
disposeFinalSpans (void)
#else
disposeFinalSpans ()
#endif
{
  struct finalSpanChunk	*chunk, *next;

  for (chunk = chunks; chunk; chunk = next) 
    {
      next = chunk->next;
      free (chunk);
    }
  chunks = (struct finalSpanChunk *)NULL;
  freeFinalSpans = (struct finalSpan *)NULL;
  free (finalSpans);
  finalSpans = (struct finalSpan **)NULL;
}

static void
#ifdef _HAVE_PROTOS
fillSpans (miGC *pGC)
#else
fillSpans (pGC)
     miGC *pGC;
#endif
{
  struct finalSpan	*span;
  miIntPoint		*xSpan;
  unsigned int		*xWidth;
  int			i;
  struct finalSpan	**f;
  int			spany;
  miIntPoint		*xSpans;
  unsigned int		*xWidths;

  if (nspans == 0)
    return;
  xSpan = xSpans = (miIntPoint *) mi_xmalloc(nspans * sizeof (miIntPoint));
  xWidth = xWidths = (unsigned int *) mi_xmalloc(nspans*sizeof (unsigned int));

  {
    i = 0;
    f = finalSpans;
    for (spany = finalMiny; spany <= finalMaxy; spany++, f++) 
      {
	for (span = *f; span; span = span->next) 
	  {
	    if (span->max <= span->min)
	      continue;
	    xSpan->x = span->min;
	    xSpan->y = spany;
	    ++xSpan;
	    *xWidth++ = (unsigned int)(span->max - span->min);
	    ++i;
	  }
      }
    miFillSpans (pGC, i, xSpans, xWidths, true);
  }
  disposeFinalSpans ();
  free (xSpans);
  free (xWidths);
  finalMiny = 0;
  finalMaxy = -1;
  finalSize = 0;
  nspans = 0;
}

#define SPAN_REALLOC	100

#define findSpan(y) ((finalMiny <= (y) && (y) <= finalMaxy) ? \
		     &finalSpans[(y) - finalMiny] : \
		     realFindSpan (y))

static struct finalSpan **
#ifdef _HAVE_PROTOS
realFindSpan (int y)
#else
realFindSpan (y)
     int y;
#endif
{
  struct finalSpan	**newSpans, **t;
  int			newSize, newMiny, newMaxy;
  int			change;
  int			i, k;

  if (y < finalMiny || y > finalMaxy) 
    {
      if (!finalSize) 
	{
	  finalMiny = y;
	  finalMaxy = y - 1;
	}
      if (y < finalMiny)
	change = finalMiny - y;
      else
	change = y - finalMaxy;
      if (change >= SPAN_REALLOC)
	change += SPAN_REALLOC;
      else
	change = SPAN_REALLOC;
      newSize = finalSize + change;

      newSpans = (struct finalSpan **)mi_xmalloc(newSize * 
						 sizeof (struct finalSpan *));
      newMiny = finalMiny;
      newMaxy = finalMaxy;
      if (y < finalMiny)
	newMiny = finalMiny - change;
      else
	newMaxy = finalMaxy + change;
      if (finalSpans) 
	{
	  memmove ((Voidptr)(newSpans + (finalMiny-newMiny)),
		   (Voidptr)finalSpans,
		   finalSize * sizeof(struct finalSpan *));
	  free (finalSpans);
	}

      if ((i = finalMiny - newMiny) > 0)
	for (k = 0, t = newSpans; k < i; k++, t++)
	  *t = (struct finalSpan *)NULL;
      if ((i = newMaxy - finalMaxy) > 0)
	for (k = 0, t = newSpans + newSize - i; k < i; k++, t++)
	  *t = (struct finalSpan *)NULL;
      finalSpans = newSpans;
      finalMaxy = newMaxy;
      finalMiny = newMiny;
      finalSize = newSize;
    }

  return &finalSpans[y - finalMiny];
}

static void
#ifdef _HAVE_PROTOS
newFinalSpan (int y, int xmin, int xmax)
#else
newFinalSpan (y, xmin, xmax)
     int y, xmin, xmax;
#endif
{
  struct finalSpan *x, *oldx, *prev, **f;

  f = findSpan (y);
  if (f == NULL)
    return;
  oldx = (struct finalSpan *)NULL;
  for (;;) 
    {
      prev = (struct finalSpan *)NULL;
      for (x = *f; x; x=x->next) 
	{
	  if (x == oldx) 
	    {
	      prev = x;
	      continue;
	    }
	  if (x->min <= xmax && xmin <= x->max) 
	    {
	      if (oldx) 
		{
		  oldx->min = IMIN (x->min, xmin);
		  oldx->max = IMAX (x->max, xmax);
		  if (prev)
		    prev->next = x->next;
		  else
		    *f = x->next;
		  --nspans;
		} 
	      else 
		{
		  x->min = IMIN (x->min, xmin);
		  x->max = IMAX (x->max, xmax);
		  oldx = x;
		}
	      xmin = oldx->min;
	      xmax = oldx->max;
	      break;
	    }
	  prev = x;
	}
      if (!x)
	break;
    }

  if (!oldx) 
    {
      x = allocFinalSpan ();
      if (x)
	{
	  x->min = xmin;
	  x->max = xmax;
	  x->next = *f;
	  *f = x;
	  ++nspans;
	}
    }
}

static void
#ifdef _HAVE_PROTOS
mirrorSppPoint (int quadrant, SppPoint *sppPoint)
#else
mirrorSppPoint (quadrant, sppPoint)
     int quadrant;
     SppPoint *sppPoint;
#endif
{
  switch (quadrant) 
    {
    case 0:
      break;
    case 1:
      sppPoint->x = -sppPoint->x;
      break;
    case 2:
      sppPoint->x = -sppPoint->x;
      sppPoint->y = -sppPoint->y;
      break;
    case 3:
      sppPoint->y = -sppPoint->y;
      break;
    }
  /*
   * and translate to X coordinate system
   */
  sppPoint->y = -sppPoint->y;
}

/* Split an arc into pieces which are scan-converted in the first quadrant
 * and mirrored into position.  This is necessary as the scan-conversion
 * code can only deal with arcs completely contained in the first quadrant.
 */
static void
#ifdef _HAVE_PROTOS
drawArc (const miArc *tarc, unsigned int l, int a0, int a1, miArcFace *right, miArcFace *left)
#else
drawArc (tarc, l, a0, a1, right, left)
     const miArc *tarc;
     unsigned int l;
     int a0, a1;
     miArcFace *right, *left;	/* these save arc endpoints */
#endif
{
  struct arc_def	def;
  struct accelerators	acc;
  int			startq, endq, curq;
  int			rightq, leftq = 0, righta = 0, lefta = 0;
  miArcFace		*passRight, *passLeft;
  int			q0 = 0, q1 = 0, mask;
  struct band 
    {
      int	a0, a1;
      int	mask;
    }			band[5], sweep[20];
  int			bandno, sweepno;
  int			i, j;
  bool			flipRight = false, flipLeft = false;
  bool			copyEnd = false;
  miArcSpanData		*spdata;
  bool			mustFree;

  /* compute span data for the whole wide ellipse, also caching it for
     speedy later retrieval */
  spdata = miComputeWideEllipse (l, tarc, &mustFree);
  if (!spdata)
    return;

  if (a1 < a0)
    a1 += 360 * 64;
  startq = a0 / (90 * 64);
  if (a0 == a1)
    endq = startq;
  else
    endq = (a1-1) / (90 * 64);
  bandno = 0;
  curq = startq;
  rightq = -1;
  for (;;) 
    {
      switch (curq) 
	{
	case 0:
	  if (a0 > 90 * 64)
	    q0 = 0;
	  else
	    q0 = a0;
	  if (a1 < 360 * 64)
	    q1 = IMIN (a1, 90 * 64);
	  else
	    q1 = 90 * 64;
	  if (curq == startq && a0 == q0 && rightq < 0) 
	    {
	      righta = q0;
	      rightq = curq;
	    }
	  if (curq == endq && a1 == q1) 
	    {
	      lefta = q1;
	      leftq = curq;
	    }
	  break;
	case 1:
	  if (a1 < 90 * 64)
	    q0 = 0;
	  else
	    q0 = 180 * 64 - IMIN (a1, 180 * 64);
	  if (a0 > 180 * 64)
	    q1 = 90 * 64;
	  else
	    q1 = 180 * 64 - IMAX (a0, 90 * 64);
	  if (curq == startq && 180 * 64 - a0 == q1) 
	    {
	      righta = q1;
	      rightq = curq;
	    }
	  if (curq == endq && 180 * 64 - a1 == q0) 
	    {
	      lefta = q0;
	      leftq = curq;
	    }
	  break;
	case 2:
	  if (a0 > 270 * 64)
	    q0 = 0;
	  else
	    q0 = IMAX (a0, 180 * 64) - 180 * 64;
	  if (a1 < 180 * 64)
	    q1 = 90 * 64;
	  else
	    q1 = IMIN (a1, 270 * 64) - 180 * 64;
	  if (curq == startq && a0 - 180*64 == q0) 
	    {
	      righta = q0;
	      rightq = curq;
	    }
	  if (curq == endq && a1 - 180 * 64 == q1) 
	    {
	      lefta = q1;
	      leftq = curq;
	    }
	  break;
	case 3:
	  if (a1 < 270 * 64)
	    q0 = 0;
	  else
	    q0 = 360 * 64 - IMIN (a1, 360 * 64);
	  q1 = 360 * 64 - IMAX (a0, 270 * 64);
	  if (curq == startq && 360 * 64 - a0 == q1) 
	    {
	      righta = q1;
	      rightq = curq;
	    }
	  if (curq == endq && 360 * 64 - a1 == q0) 
	    {
	      lefta = q0;
	      leftq = curq;
	    }
	  break;
	}
      band[bandno].a0 = q0;
      band[bandno].a1 = q1;
      band[bandno].mask = 1 << curq;
      bandno++;
      if (curq == endq)
	break;
      curq++;
      if (curq == 4) 
	{
	  a0 = 0;
	  a1 -= 360 * 64;
	  curq = 0;
	  endq -= 4;
	}
    }
  sweepno = 0;
  for (;;) 
    {
      q0 = 90 * 64;
      mask = 0;
      /*
       * find left-most point
       */
      for (i = 0; i < bandno; i++)
	if (band[i].a0 <= q0) 
	  {
	    q0 = band[i].a0;
	    q1 = band[i].a1;
	    mask = band[i].mask;
	  }
      if (!mask)
	break;
      /*
       * locate next point of change
       */
      for (i = 0; i < bandno; i++)
	if (!(mask & band[i].mask)) 
	  {
	    if (band[i].a0 == q0) 
	      {
		if (band[i].a1 < q1)
		  q1 = band[i].a1;
		mask |= band[i].mask;
	      } 
	    else if (band[i].a0 < q1)
	      q1 = band[i].a0;
	  }
      /*
       * create a new sweep
       */
      sweep[sweepno].a0 = q0;
      sweep[sweepno].a1 = q1;
      sweep[sweepno].mask = mask;
      sweepno++;
      /*
       * subtract the sweep from the affected bands
       */
      for (i = 0; i < bandno; i++)
	if (band[i].a0 == q0) 
	  {
	    band[i].a0 = q1;
	    /*
	     * check if this band is empty
	     */
	    if (band[i].a0 == band[i].a1)
	      band[i].a1 = band[i].a0 = 90 * 64 + 1;
	  }
    }
  computeAcc (tarc, l, &def, &acc);
  for (j = 0; j < sweepno; j++) 
    {
      mask = sweep[j].mask;
      passRight = passLeft = (miArcFace *)NULL;
      if (mask & (1 << rightq)) 
	{
	  if (sweep[j].a0 == righta)
	    passRight = right;
	  else if (sweep[j].a1 == righta) 
	    {
	      passLeft = right;
	      flipRight = true;
	    }
	}
      if (mask & (1 << leftq)) 
	{
	  if (sweep[j].a1 == lefta)
	    {
	      if (passLeft)
		copyEnd = true;
	      passLeft = left;
	    }
	  else if (sweep[j].a0 == lefta) 
	    {
	      if (passRight)
		copyEnd = true;
	      passRight = left;
	      flipLeft = true;
	    }
	}

      drawQuadrant (&def, &acc, sweep[j].a0, sweep[j].a1, mask, 
		    passRight, passLeft, spdata);
    }

  /* when copyEnd is true, both ends of the arc were computed at the same
   * time; drawQuadrant only takes one end though, so the left end will be
   * the only one holding the data.  Copy it from there.
   */
  if (copyEnd)
    *right = *left;
  /*
   * mirror the coordinates generated for the
   * faces of the arc
   */
  if (right) 
    {
      mirrorSppPoint (rightq, &right->clock);
      mirrorSppPoint (rightq, &right->center);
      mirrorSppPoint (rightq, &right->counterClock);
      if (flipRight) 
	{
	  SppPoint	temp;

	  temp = right->clock;
	  right->clock = right->counterClock;
	  right->counterClock = temp;
	}
    }
  if (left) 
    {
      mirrorSppPoint (leftq,  &left->counterClock);
      mirrorSppPoint (leftq,  &left->center);
      mirrorSppPoint (leftq,  &left->clock);
      if (flipLeft) 
	{
	  SppPoint	temp;

	  temp = left->clock;
	  left->clock = left->counterClock;
	  left->counterClock = temp;
	}
    }
  if (mustFree)
    free (spdata);
}

static void
#ifdef _HAVE_PROTOS
drawQuadrant (struct arc_def *def, struct accelerators *acc, int a0, int a1, int mask, miArcFace *right, miArcFace *left, miArcSpanData *spdata)
#else
drawQuadrant (def, acc, a0, a1, mask, right, left, spdata)
     struct arc_def *def;
     struct accelerators *acc;
     int a0, a1;
     int mask;
     miArcFace *right, *left;
     miArcSpanData *spdata;
#endif
{
  struct arc_bound	bound;
  double		yy, x, xalt;
  int			y, miny, maxy;
  int			n;
  miArcSpan		*span;

  def->a0 = ((double) a0) / 64.0;
  def->a1 = ((double) a1) / 64.0;

  computeBound (def, &bound, acc, right, left);
  yy = bound.inner.min;
  if (bound.outer.min < yy)
    yy = bound.outer.min;
  miny = ICEIL(yy - acc->fromIntY);
  yy = bound.inner.max;
  if (bound.outer.max > yy)
    yy = bound.outer.max;
  maxy = (int)floor(yy - acc->fromIntY);
  y = spdata->k;
  span = spdata->spans;
  if (spdata->top)
    {
      if (a1 == 90 * 64 && (mask & 1))
	newFinalSpan (acc->yorgu - y - 1, acc->xorg, acc->xorg + 1);
      span++;
    }
  for (n = spdata->count1; --n >= 0; )
    {
      if (y < miny)
	return;
      if (y <= maxy) 
	{
	  /* generate set of spans at this y value */
	  arcSpan (y,
		   span->lx, -span->lx, 0, span->lx + span->lw,
		   def, &bound, acc, mask);
	  if (span->rw + span->rx)
	    tailSpan (y, -span->rw, -span->rx, def, &bound, acc, mask);
	}
      y--;
      span++;
    }
  if (y < miny)
    return;
  if (spdata->hole)
    {
      if (y <= maxy)
	/* generate set of spans at this y value */
	arcSpan (y, 0, 0, 0, 1, 
		 def, &bound, acc, mask & 0xc);
    }

  for (n = spdata->count2; --n >= 0; )
    {
      if (y < miny)
	return;
      if (y <= maxy)
	/* generate set of spans at this y value */
	arcSpan (y, span->lx, span->lw, span->rx, span->rw,
		 def, &bound, acc, mask);
      
      y--;
      span++;
    }

  if (spdata->bot && miny <= y && y <= maxy)
    {
      n = mask;
      if (y == miny)
	n &= 0xc;
      if (span->rw <= 0) 
	{
	  arcSpan0 (span->lx, -span->lx, 0, span->lx + span->lw,
		    def, &bound, acc, n);
	  if (span->rw + span->rx)
	    tailSpan (y, -span->rw, -span->rx, def, &bound, acc, n);
	}
      else
	arcSpan0 (span->lx, span->lw, span->rx, span->rw,
		  def, &bound, acc, n);
      y--;
    }

  while (y >= miny) 
    {
      yy = y + acc->fromIntY;
      if (def->w == def->h) 
	{
	  xalt = def->w - def->l;
	  x = -sqrt(xalt * xalt - yy * yy);
	} 
      else 
	{
	  x = tailX(yy, def, &bound, acc);
	  if (acc->left.valid && boundedLe (yy, bound.left)) 
	    {
	      xalt = intersectLine (yy, acc->left);
	      if (xalt < x)
		x = xalt;
	    }
	  if (acc->right.valid && boundedLe (yy, bound.right)) 
	    {
	      xalt = intersectLine (yy, acc->right);
	      if (xalt < x)
		x = xalt;
	    }
	}
      /* generate set of spans at this y value */
      arcSpan (y,
	       ICEIL(acc->fromIntX - x), 0,
	       ICEIL(acc->fromIntX + x), 0,
	       def, &bound, acc, mask);
      y--;
    }
}
