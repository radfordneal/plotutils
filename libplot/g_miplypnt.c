#include "sys-defines.h"
#include "extern.h"

#include "g_mi.h"

void
#ifdef _HAVE_PROTOS
miPolyPoint(miGC *pGC, int mode, int npt, miIntPoint *pPts)
#else
miPolyPoint(pGC, mode, npt, pPts)
     miGC *pGC;
     int mode;			/* mode = Origin or Previous */
     int npt;
     miIntPoint *pPts;
#endif
{

  int 		nptTmp;
  int		fsOld;
  unsigned int	*pwidthInit, *pwidth;
  int		i;
  miIntPoint 	*ppt;

  /* make pointlist origin relative */
  if (mode == miCoordModePrevious)
    {
      ppt = pPts;
      nptTmp = npt;
      nptTmp--;
      while(nptTmp--)
	{
	  ppt++;
	  ppt->x += (ppt-1)->x;
	  ppt->y += (ppt-1)->y;
	}
    }

  fsOld = pGC->fillStyle;
  if(fsOld != miFillSolid)
    pGC->fillStyle = miFillSolid;	/* ChangeGC */

  pwidthInit = (unsigned int *)mi_xmalloc (npt * sizeof(unsigned int));
  pwidth = pwidthInit;
  for(i = 0; i < npt; i++)
    *pwidth++ = 1;
  miFillSpans (pGC, npt, pPts, pwidthInit, false); 

  if(fsOld != miFillSolid)
    pGC->fillStyle = fsOld;
  free (pwidthInit);
}

