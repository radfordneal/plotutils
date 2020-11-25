/* The inmost pixel-painting routine for the MI scan conversion module.
   All pixel painting in the module goes through this.  This is where all
   clipping gets done, except that the Bresenham line-drawing routines in
   g_mizerolin.c, miZeroLine and miZeroDash, do their own clipping before
   this is even invoked. */

/* 0. The FillSolid fill style draws and fills objects using the
   `foreground' color of the drawing state.  Exception: double-dashed
   polylines, which are drawn by invoking miWideDash() or miZeroDash().
   They are drawn alternately in the foreground and background colors.

   1. FillStippled is screendoor patterning, with a stipple bitmap used as
   a mask, indicating where pixels are allowed to be painted [normally in
   the `foreground' color].  1=paint, 0=don't paint.

   2. FillOpaqueStippled is hatch patterning, with a stipple bitmap
   specifying coloring in foreground/background colors.  

   3. FillTiled tiles with an arbitrarily colored tiling pixmap.  

   For the final two, drawing of double-dashed lines make no sense (if the
   user tries it, solid, i.e. non-dashed lines are used instead).  Also
   drawing of isolated points, via miPolyPoint(), is always done in the
   FillSolid fill style. */

#include "sys-defines.h"
#include "extern.h"

#include "g_mi.h"

void 
#ifdef _HAVE_PROTOS
miFillSpans (miGC *pGC, int n, const miIntPoint *ppt, const unsigned int *pwidth, bool sorted)
#else
miFillSpans (pGC, n, ppt, pwidth, sorted)
     miGC *pGC;
     int n;
     const miIntPoint *ppt;
     const unsigned int *pwidth;
     bool sorted;		/* y vals of points are non-decreasing? */
#endif
{
  int i;
  int fillStyle;
  int xorigin = 0, yorigin = 0;
  int xleft, xright, ybottom, ytop;
  miPixel fgPixel, bgPixel;
  unsigned int stippleWidth, stippleHeight;
  unsigned int tileWidth, tileHeight;

  int xstart, xend, xstart_clip, xend_clip, x, y;
  unsigned int width;

  /* bounds of drawable, i.e., 2-D pixel array */
  xleft   = 0;
  ytop    = 0;
  xright  = pGC->width - 1;
  ybottom = pGC->height - 1;

  /* if no overlap with drawable, do nothing */
  if (sorted && (ppt[0].y > ybottom || ppt[n-1].y < ytop))
    return;

  fillStyle = pGC->fillStyle;

  switch (fillStyle)
    {
    case miFillSolid:
    default:
      
      fgPixel = pGC->fgPixel;
      for (i = 0; i < n; i++)
	{
	  y = ppt[i].y;
	  if (sorted && y > ybottom)
	    return;
	  if (y >= ytop && y <= ybottom)
	    {
	      width = pwidth[i];
	      xstart = ppt[i].x;
	      xend = xstart + (int)width - 1;

	      xstart_clip = IMAX(xstart,xleft);
	      xend_clip = IMIN(xend,xright);
	      for (x = xstart_clip; x <= xend_clip; x++) /* can be empty */
		pGC->drawable[y][x] = fgPixel;
	    }
	}
      break;

    case miFillStippled:
    case miFillOpaqueStippled:

      fgPixel = pGC->fgPixel;
      bgPixel = pGC->bgPixel;
      stippleWidth = pGC->stippleWidth;
      stippleHeight = pGC->stippleHeight;  
      xorigin = pGC->stippleXorigin;
      yorigin = pGC->stippleYorigin;
      while (xorigin > 0)
	xorigin -= stippleWidth;
      while (yorigin > 0)
	yorigin -= stippleHeight;

      for (i = 0; i < n; i++)
	{
	  y = ppt[i].y;
	  if (sorted && y > ybottom)
	    return;
	  if (y >= ytop && y <= ybottom)
	    {
	      width = pwidth[i];
	      xstart = ppt[i].x;
	      xend = xstart + (int)width - 1;

	      xstart_clip = IMAX(xstart,xleft);
	      xend_clip = IMIN(xend,xright);

	      for (x = xstart_clip; x <= xend_clip; x++) /* can be empty */
		{
		  if (pGC->stipple[(y-yorigin) % stippleHeight][(x-xorigin) % stippleWidth])
		    pGC->drawable[y][x] = fgPixel;
		  else if (fillStyle == miFillOpaqueStippled)
		    pGC->drawable[y][x] = bgPixel;
		}
	    }
	}
      break;

    case miFillTiled:
      tileWidth = pGC->tileWidth;
      tileHeight = pGC->tileHeight;  
      xorigin = pGC->tileXorigin;
      yorigin = pGC->tileYorigin;
      while (xorigin > 0)
	xorigin -= tileWidth;
      while (yorigin > 0)
	yorigin -= tileHeight;

      for (i = 0; i < n; i++)
	{
	  y = ppt[i].y;
	  if (sorted && y > ybottom)
	    return;
	  if (y >= ytop && y <= ybottom)
	    {
	      width = pwidth[i];
	      xstart = ppt[i].x;
	      xend = xstart + (int)width - 1;

	      xstart_clip = IMAX(xstart,xleft);
	      xend_clip = IMIN(xend,xright);
	      for (x = xstart_clip; x <= xend_clip; x++) /* can be empty */
		pGC->drawable[y][x] = 
		  pGC->tile[(y-yorigin) % tileHeight][(x-xorigin) % tileWidth];
	    }
	}
      break;

    } /* end switch */
}
