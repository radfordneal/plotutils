/* This file contains the space method, which is a standard part of
   libplot.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a fixed
   square.  The arguments to the space method are the lower left and upper
   right vertices of a `window' (a drawing rectangle), in user coordinates.
   This window, whose axes are aligned with the coordinate axes, will be
   mapped affinely onto the square on the display device.

   This file also contains the space2 method, which is a GNU extension to
   libplot.  The arguments to the space2 method are the vertices of an
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the square on the
   display device.

   Invoking space (or space2, etc.) causes the default line width and
   default font size, as expressed in user units, to be recomputed.  That
   is because those two quantities are specified as a fraction of the size
   of the physical display: in device terms, rather than in terms of user
   units. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* potential roundoff error (absolute, for defining boundary of display) */
#define ROUNDING_FUZZ 0.0000001
/* potential roundoff error (relative, used for checking isotropy etc.) */
#define FUZZ 0.0000001

/* The vertices of the parallelogram in user space have coordinates (going
   counterclockwise) (x0,y0), (x1,y1), (x1,y1)+(x2,y2)-(x0,y0), and
   (x2,y2). */

int
#ifdef _HAVE_PROTOS
_g_fspace2 (double x0, double y0, double x1, double y1, double x2, double y2)
#else
_g_fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
#endif
{
  double s[6], t[6];
  double v0x, v0y, v1x, v1y, v2x, v2y;
  double cross;
  double norm;
  double device_x_left, device_x_right, device_y_bottom, device_y_top;

  if (!_plotter->open)
    {
      _plotter->error ("fspace2: invalid operation");
      return -1;
    }

  /* First, compute affine transformation from user frame to NDC [normalized
     device coordinates] frame.  The parallelogram in the user frame is
     mapped to the square [0,1]x[0,1] in the NCD frame.  */

  v0x = x0;
  v0y = y0;
  v1x = x1 - x0;
  v1y = y1 - y0;
  v2x = x2 - x0;
  v2y = y2 - y0;
  cross = v1x * v2y - v1y * v2x;

  if (cross == 0.0) 
    {
      _plotter->error ("cannot perform singular affine transformation");
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
  
  /* Second, compute the affine transformation from the NCD frame to the
     device frame (the square [0,1]x[0,1] in the NCD frame is mapped to the
     ``graphics display'' [a specified square region in device space]). */

  if (_plotter->bitmap_device)
    {
      /* test whether flipped-y convention is used */
      double sign = (_plotter->jmin < _plotter->jmax ? 1.0 : -1.0);

      device_x_left = (double)(_plotter->imin) - 0.5 + ROUNDING_FUZZ;
      device_x_right = (double)(_plotter->imax) + 0.5 - ROUNDING_FUZZ;
      device_y_bottom = (double)(_plotter->jmin)
	+ sign * (- 0.5 + ROUNDING_FUZZ);
      device_y_top = (double)(_plotter->jmax) 
	+ sign * (0.5 - ROUNDING_FUZZ);
    }
  else	/* a physical device, dimensions are specified in inches */
    {
      device_x_left = (_plotter->device_units_per_inch 
		       *_plotter->display_coors.left);
      device_x_right = (_plotter->device_units_per_inch 
		       *_plotter->display_coors.right);
      device_y_bottom = (_plotter->device_units_per_inch 
		       *_plotter->display_coors.bottom);
      device_y_top = (_plotter->device_units_per_inch 
		       *_plotter->display_coors.top);
    }

  /* linear transformation */
  t[0] = device_x_right - device_x_left;
  t[1] = t[2] = 0.0;
  t[3] = device_y_top - device_y_bottom;
  
  /* translation */
  t[4] = device_x_left;
  t[5] = device_y_bottom;

  /* Compute the affine transformation from the user frame to the device
     frame, as the product of affine transformations #1 and #2. */
  _matrix_product (s, t, _plotter->drawstate->transform.m);

  /* does map preserve axis directions? */
  _plotter->drawstate->transform.axes_preserved = 
    (_plotter->drawstate->transform.m[1] == 0.0 
     && _plotter->drawstate->transform.m[2] == 0.0);

#define IS_ZERO(arg) (IS_ZERO1(arg) && IS_ZERO2(arg))
#define IS_ZERO1(arg) (fabs(arg) < FUZZ * DMAX(_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[0], _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[1]))
#define IS_ZERO2(arg) (fabs(arg) < FUZZ * DMAX(_plotter->drawstate->transform.m[2] * _plotter->drawstate->transform.m[2], _plotter->drawstate->transform.m[3] * _plotter->drawstate->transform.m[3]))
  /* if row vectors are of equal length and orthogonal... */
  if (IS_ZERO(_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[0]
	      + _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[1]
	      - _plotter->drawstate->transform.m[2] * _plotter->drawstate->transform.m[2]
	      - _plotter->drawstate->transform.m[3] * _plotter->drawstate->transform.m[3])
      &&
      IS_ZERO(_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[2] + 
	      _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[3]))
    _plotter->drawstate->transform.uniform = true; /* map's scaling is uniform */
  else
    _plotter->drawstate->transform.uniform = false; /* map's scaling not uniform */

  /* determine whether map involves a reflection, by computing determinant */
  {
    double det;
    
    det = (_plotter->drawstate->transform.m[0] *
	   _plotter->drawstate->transform.m[3]
	   - (_plotter->drawstate->transform.m[1] *
	      _plotter->drawstate->transform.m[2]));
    _plotter->drawstate->transform.nonreflection 
      = ((_plotter->flipped_y ? -1 : 1) * det >= 0);
  }
  
  /* Compute matrix norm of linear transformation appearing in the affine
     map from the user frame to the NCD frame. */
  norm = _matrix_norm (s);

  /* We want to maintain backwards compatibility with traditional libplot,
     where the user is not required to make initial calls to linewidth()
     and fontsize(), but is required to make a single initial call to
     space().  So when the user calls space() [or space2() or fspace() or
     fspace2()], we set the line width and font size to something reasonable.

     We also set the default line width and font size, as stored in the
     _plotter->default_drawstate structure, to something reasonable.  The
     default values stored there will be used later, if the user calls
     linewidth() or fontsize() with out-of-bound arguments. */

  /* Incidentally, invoking ffontsize will invoke the internal
     retrieve_font() method.  So e.g. if the Plotter is an XPlotter then
     invoking ffontsize() will retrieve an X font from the X server. */

  _plotter->default_drawstate->line_width 
    = DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH / norm;
  _plotter->default_drawstate->font_size
    = DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH / norm;

  _plotter->flinewidth (_plotter->default_drawstate->line_width);
  _plotter->ffontsize (_plotter->default_drawstate->font_size);

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_fspace (double x0, double y0, double x1, double y1)
#else
_g_fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  return _plotter->fspace2 (x0, y0, x1, y0, x0, y1);
}
