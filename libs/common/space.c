/* This file contains the space routine, which is a standard part of
   libplot.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a square.
   The arguments to the space routine are the lower left and upper right
   vertices of a `window' (a drawing rectangle), in user coordinates.  This
   window, whose axes are aligned with the coordinate axes, will be mapped
   affinely onto the square on the display device.

   This file also contains the space2 routine, which is a GNU extension to
   libplot.  The arguments to the space2 routine are the vertices of an
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the square on the
   display device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* The vertices of the parallelogram in user space have coordinates (going
   counterclockwise) (x0,y0), (x1,y1), (x1,y1)+(x2,y2)-(x0,y0), and
   (x2,y2). */

int
fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
{
  double s[6], t[6];
  double v0x, v0y, v1x, v1y, v2x, v2y;
  double cross;
  double norm;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fspace() called when graphics device not open\n");
      return -1;
    }

  /* First compute affine transformation from user frame to NDC [normalized
     device coordinates] frame.  The parallelogram is mapped to the square
     [0,1]x[0,1].  */

  v0x = x0;
  v0y = y0;
  v1x = x1 - x0;
  v1y = y1 - y0;
  v2x = x2 - x0;
  v2y = y2 - y0;
  cross = v1x * v2y - v1y * v2x;

  if (cross == 0.0) 
    {
      fprintf (stderr, "libplot: fspace2(): cannot perform singular affine transformation\n");
      return -1;
    }

  /* linear transformation */  
  s[0] = v2y / cross;
  s[1] = -v1y / cross;
  s[2] = -v2x / cross;
  s[3] = v1x / cross;

  /* translation */
  s[4] = - (v0x * v2y - v0y * v2x) / cross;
  s[5] = (v0x * v1y - v0y * v1x) / cross;
  
  /* next the affine transformation from normalized device coordinates to
     actual device coordinates (the square [0,1]x[0,1] is mapped to the
     square in device space) */

  /* linear transformation */
  t[0] = DEVICE_X_RIGHT - DEVICE_X_LEFT;
  t[1] = t[2] = 0.0;
  t[3] = DEVICE_Y_TOP - DEVICE_Y_BOTTOM;
  
  /* translation */
  t[4] = DEVICE_X_LEFT;
  t[5] = DEVICE_Y_BOTTOM;

  /* compute product of transformations #1 and #2 */
  _matrix_product (s, t, _drawstate->transform.m);

  /* does map preserve axis directions? */
  _drawstate->transform.axes_preserved = 
    (_drawstate->transform.m[1] == 0.0 
     && _drawstate->transform.m[2] == 0.0);

#define FUZZ 0.0000001		/* potential roundoff error */
#define IS_ZERO(arg) (IS_ZERO1(arg) && IS_ZERO2(arg))
#define IS_ZERO1(arg) (fabs(arg) < FUZZ * DMAX(_drawstate->transform.m[0] * _drawstate->transform.m[0], _drawstate->transform.m[1] * _drawstate->transform.m[1]))
#define IS_ZERO2(arg) (fabs(arg) < FUZZ * DMAX(_drawstate->transform.m[2] * _drawstate->transform.m[2], _drawstate->transform.m[3] * _drawstate->transform.m[3]))
  /* if row vectors are of equal length and orthogonal... */
  if (IS_ZERO(_drawstate->transform.m[0] * _drawstate->transform.m[0]
	      + _drawstate->transform.m[1] * _drawstate->transform.m[1]
	      - _drawstate->transform.m[2] * _drawstate->transform.m[2]
	      - _drawstate->transform.m[3] * _drawstate->transform.m[3])
      &&
      IS_ZERO(_drawstate->transform.m[0] * _drawstate->transform.m[2] + 
	      _drawstate->transform.m[1] * _drawstate->transform.m[3]))
    _drawstate->transform.uniform = TRUE; /* map's scaling is uniform */
  else
    _drawstate->transform.uniform = FALSE; /* map's scaling not uniform */

  /* Compute matrix norm of linear transformation appearing in the affine
     user->NCD map. */
  norm = _matrix_norm (s);

  /* We want to maintain backwards compatibility with traditional libplot,
     where the user is not required to make initial calls to linewidth()
     and fontsize(), but is required to make a single initial call to
     space().  So when the user calls space() [or space2() or fspace() or
     fspace2()], we set the default line width and font size, as stored in
     the _default_drawstate structure, to something reasonable. */

  _default_drawstate.line_width 
    = DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH / norm;
  _default_drawstate.font_size
    = DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH / norm;

  flinewidth (_default_drawstate.line_width);
  ffontsize (_default_drawstate.font_size);

  return 0;
}

int
fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
{
  return fspace2 (x0, y0, x1, y0, x0, y1);
}

int
space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
{
  return fspace ((double)x0, (double)y0, (double)x1, (double)y1);
}

int
space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
{
  return fspace2 ((double)x0, (double)y0, (double)x1, (double)y1, 
		  (double)x2, (double)y2);
}
