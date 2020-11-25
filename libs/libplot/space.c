/* This file contains the space routine, which is a standard part of
   libplot.  library.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a square.
   The arguments to the space routine are the lower left and upper right
   vertices of a `window' (a drawing rectangle), in user coordinates.  This
   window, whose axes are aligned with the coordinate axes, will be mapped
   affinely onto the square on the display device.

   This file also contains the space2 routine, which is a GNU extension to
   libplot.  The arguments to the space routine are the vertices of a of a
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the square on the
   display device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: space() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d\n", 
	     SPACE, x0, y0, x1, y1);
  else
    {
      putc (SPACE, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
    }
  
  return 0;
}

int
fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fspace() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g\n", 
	     FSPACE, x0, y0, x1, y1);
  else
    {
      putc (FSPACE, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
    }
  
  return 0;
}

int
space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: space2() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d %d %d %d %d\n", 
	     SPACE2, x0, y0, x1, y1, x2, y2);
  else
    {
      putc (SPACE, _outstream);
      _putshort (x0, _outstream);
      _putshort (y0, _outstream);
      _putshort (x1, _outstream);
      _putshort (y1, _outstream);
      _putshort (x2, _outstream);
      _putshort (y2, _outstream);
    }
  
  return 0;
}

int
fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fspace2() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g %g\n", 
	     FSPACE2, x0, y0, x1, y1, x2, y2);
  else
    {
      putc (FSPACE2, _outstream);
      _putfloat (x0, _outstream);
      _putfloat (y0, _outstream);
      _putfloat (x1, _outstream);
      _putfloat (y1, _outstream);
      _putfloat (x2, _outstream);
      _putfloat (y2, _outstream);
    }
  
  return 0;
}
