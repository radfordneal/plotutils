/* This file contains the fill routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the desaturation level of the
   filling, for all objects created by the drawing operations that follow.
   (For those that can be filled, that is; text cannot be filled.)

   The argument to FILL ranges from 0 to 0xFFFF.  The value 0 is special;
   it signifies no filling at all (the object will be transparent).  The
   value 1 signifies that the fill color should be the user-specified fill
   color, and a value of 0xFFFF signifies complete desaturation of this
   color (i.e., white).  Values intermediate between 1 and 0xFFFF yield
   intermediate saturations of the user-specified fill color.  An
   out-of-bounds argument resets the desaturation level to a default
   value. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fill (level)
     int level;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fill() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  _drawstate->fill_level = level;
  
  return 0;
}
