#include "sys-defines.h"
#include "extern.h"

#include "g_mi.h"

/*
 * Written by Brian Kelleher; June 1986
 *
 * Draw a polygon (supplied as a polyline, i.e. an array of points), via
 * one of two scan conversion routines.
 */

void
#ifdef _HAVE_PROTOS
miFillPolygon(miGC *pGC, int shape, int mode, int count, miIntPoint *pPts)
#else
miFillPolygon(pGC, shape, mode, count, pPts)
     miGC *pGC;
     int shape;
     int mode;
     int count;
     miIntPoint *pPts;
#endif
{
  miIntPoint *ppt;
    
  if (count == 0)
    return;

  ppt = pPts;
  {
    if (mode == miCoordModePrevious)
      /* convert from relative to absolute coordinates */
      {
	int i;

	ppt++;
	for (i = 1; i < count; i++) 
	  {
	    ppt->x += (ppt-1)->x;
	    ppt->y += (ppt-1)->y;
	    ppt++;
	  }
      }
  }
  switch (shape)
    {
    case (int)miComplex:
    default:
      /* use general scan conversion routine */
      miFillGeneralPoly (pGC, count, pPts);
      break;
    case (int)miConvex:
      /* use special (faster) routine */
      miFillConvexPoly (pGC, count, pPts);
      break;
    }
}


