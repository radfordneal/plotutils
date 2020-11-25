/* This file contains the versions of the fontname, fontsize, and textangle
   routines that (i) take as arguments, and/or (ii) return, integers rather
   than doubles.

   Return value for all three of these is the size, in user coordinates, of
   the newly selected font. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int 
fontname (s)
     char *s;
{
  double new_size = ffontname (s);
  
  return IROUND(new_size);
}

int
fontsize (size)
     int size;
{
  double new_size = ffontsize ((double)size);

  return IROUND(new_size);
}

int
textangle (angle)
     int angle;
{
  double new_size = ftextangle ((double)angle);

  return IROUND(new_size);
}
