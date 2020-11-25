/* This file contains device-specific color database access routines.
   These routines are called by functions in libplottek. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* _kermit_pseudocolor quantizes to one of kermit's native 16 colors.
   (They provide a [rather strange] partition of the color cube; see
   colordb.h.) */

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
int _kermit_pseudocolor P__((int red, int green, int blue));
#undef P__

#define ONEBYTE (0xff)

/* find closest known point within the RGB color cube, using Euclidean
   distance as our metric */
int
_kermit_pseudocolor (red, green, blue)
     int red, green, blue;
{
  unsigned long int difference = MAXINT;
  int i;
  int best = 0;
  
  /* reduce to 24 bits */
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < KERMIT_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      newdifference = (_kermit_stdcolors[i].red - red) * (_kermit_stdcolors[i].red - red)
	+ (_kermit_stdcolors[i].green - green) * (_kermit_stdcolors[i].green - green) 
	  + (_kermit_stdcolors[i].blue - blue) * (_kermit_stdcolors[i].blue - blue);
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return best;
}

