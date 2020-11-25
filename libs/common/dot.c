/* This file contains the dot routine, which is a standard (though
   undocumented and obsolete) part of libplot.  I haven't yet found out
   exactly what `dot' is supposed to do, or ever did, so it is not
   implemented fully.  It appears from the source to the BSD version of
   plot(3) that it was specific to a single class of devices (Versatec
   plotters), and in fact to a single driver for those devices (/dev/vt0,
   an in-kernel driver).

   According to Jens-Uwe Mager <jum@anubis.han.de>, /dev/vt0 was a driver
   for a Unibus expansion card used on PDP-11's; the card provided the
   interface to a Versatec.  The driver apparently had a virtual pixel
   space of size 0..4095 x 0..4095, and accepted simple commands like the
   following (op code is one byte, args are short 2-byte ints unless
   otherwise noted):

     1 3 string		    draw label, '3'=1-byte op code, string has \0
     2 x y		    draw point at (x,y)
     3 x0 x0 x1 y1	    draw line from (x0,y0) to (x1,y1)
     5 x y r		    draw circle centered on (x,y), radius r
     6 xc yc x0 y0 x1 y1    draw arc from (x0,y0) to (x1,y1) center (xc,yc) 
     7 x y dx n pattern	    draw dot, size dx (pattern is n>0 bytes long,
     				n=0 replaced by n=256)

   Unfortunately it's not clear what a dot was.  Maybe drawing a dot filled
   a circle (or a square) with raster data?  Or did raster fill?  Jens-Uwe
   recalls seeing the driver in the kernel source for System V.0 on a VAX
   in the early 1980's. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
dot (x, y, dx, n, pattern)
     int x, y, dx, n;
     char *pattern;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: dot() called when graphics device not open\n");
      return -1;
    }

  point (x, y);
  return 0;
}
