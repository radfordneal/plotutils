#include "sys-defines.h"
#include "extern.h"

#include "g_mi.h"

/* mi rectangles
   written by Todd Newman, with debts to all and sundry
   */

/* MIPOLYFILLRECT -- public entry for PolyFillRect request.  Very
 * straightforward: translate rectangles if necessary, then call
 * miFillSpans to fill each rectangle.  We let miFillSpans worry about
 * clipping to the destination.
 */

void
#ifdef _HAVE_PROTOS
miPolyFillRect(miGC *pGC, int nrectFill, const miRectangle *prectInit)
#else
miPolyFillRect(pGC, nrectFill, prectInit)
     miGC *pGC;
     int nrectFill; 
     const miRectangle *prectInit;
#endif
/* nrectFill = num of rects to fill, prectInit = pointer to 1st of them */
{
  miIntPoint *ppt;
  miIntPoint *pptFirst;
  int i;
  int xorg, yorg;
  unsigned int *pw, *pwFirst;
  unsigned int height, width;
  unsigned int maxheight;
  const miRectangle *prect; 

  prect = prectInit;
  maxheight = 0;
  for (i = 0; i < nrectFill; i++, prect++)
    maxheight = IMAX(maxheight, prect->height);

  pptFirst = (miIntPoint *)mi_xmalloc (maxheight * sizeof(miIntPoint));
  pwFirst = (unsigned int *)mi_xmalloc (maxheight * sizeof(unsigned int));
  prect = prectInit;

  while (nrectFill--)
    {
      ppt = pptFirst;
      pw = pwFirst;
      height = prect->height;
      width = prect->width;
      xorg = prect->x;
      yorg = prect->y;
      while (height--)
	{
	  *pw++ = width;
	  ppt->x = xorg;
	  ppt->y = yorg;
	  ppt++;
	  yorg++;
	}
      miFillSpans(pGC, (int)(prect->height), pptFirst, pwFirst, true);
      prect++;
    }
  free (pwFirst);
  free (pptFirst);
}
