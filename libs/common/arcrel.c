/* This file contains the arcrel() routine, which is a GNU extension to
   libplot.  arcrel() is simply a version of arc() that uses relative
   coordinates.  It draws an object: a counterclockwise arc with specified
   center, beginning, and end.

   This file also contains the ellarcrel() routine, which is an extension
   to libplot. It draws an object: an elliptical arc. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
farcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     double dxc, dyc, dx0, dy0, dx1, dy1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: farcrel() called when graphics device not open\n");
      return -1;
    }

  farc ((_drawstate->pos).x + dxc, (_drawstate->pos).y + dyc,
       (_drawstate->pos).x + dx0, (_drawstate->pos).y + dy0,
       (_drawstate->pos).x + dx1, (_drawstate->pos).y + dy1);
  
  return 0;
}

int
arcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     int dxc, dyc, dx0, dy0, dx1, dy1;
{
  return farcrel ((double)dxc, (double)dyc, 
		  (double)dx0, (double)dy0, 
		  (double)dx1, (double)dy1);
}

int
fellarcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     double dxc, dyc, dx0, dy0, dx1, dy1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: farcrel() called when graphics device not open\n");
      return -1;
    }

  fellarc ((_drawstate->pos).x + dxc, (_drawstate->pos).y + dyc,
	   (_drawstate->pos).x + dx0, (_drawstate->pos).y + dy0,
	   (_drawstate->pos).x + dx1, (_drawstate->pos).y + dy1);
  
  return 0;
}

int
ellarcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     int dxc, dyc, dx0, dy0, dx1, dy1;
{
  return fellarcrel ((double)dxc, (double)dyc, 
		     (double)dx0, (double)dy0, 
		     (double)dx1, (double)dy1);
}

    
