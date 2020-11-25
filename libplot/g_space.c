/* This file contains the space method, which is a standard part of
   libplot.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a fixed
   rectangle (usually a square).  The arguments to the space method are the
   lower left and upper right vertices of a `window' (a drawing rectangle),
   in user coordinates.  This window, whose axes are aligned with the
   coordinate axes, will be mapped affinely onto the drawing region on the
   display device.

   This file also contains the space2 method, which is a GNU extension to
   libplot.  The arguments to the space2 method are the vertices of an
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the drawing region on
   the display device.

   Invoking space (or space2, etc.) causes the default line width and
   default font size, as expressed in user units, to be recomputed.  That
   is because those two quantities are specified as a fraction of the size
   of the display: in device terms, rather than in terms of user units.
   The idea is that no matter what the arguments of space (etc.) were,
   switching later to the default line width or default font size, by
   e.g. passing an out-of-bounds argument to linewidth() or fontsize(),
   should yield something reasonable. */

#include "sys-defines.h"
#include "extern.h"

/* possible rotation angles for viewport on output device */
enum rotation_type { ROT_0, ROT_90, ROT_180, ROT_270 };

/* potential roundoff error (absolute, for defining boundary of display) */
#define ROUNDING_FUZZ 0.0000001
/* potential roundoff error (relative, used for checking isotropy etc.) */
#define FUZZ 0.0000001

/* The vertices of the parallelogram in user space have coordinates (going
   counterclockwise) (x0,y0), (x1,y1), (x1,y1)+(x2,y2)-(x0,y0), and
   (x2,y2). */

int
#ifdef _HAVE_PROTOS
_g_fspace2 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2)
#else
_g_fspace2 (R___(_plotter) x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1, x2, y2;
#endif
{
  double s[6], t[6];
  double v0x, v0y, v1x, v1y, v2x, v2y;
  double cross;
  double norm, min_sing_val, max_sing_val;
  double device_x_left, device_x_right, device_y_bottom, device_y_top;
  const char *rotation_s;
  int rotation_angle;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fspace2: invalid operation");
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
      _plotter->error (R___(_plotter) 
		       "cannot perform singular affine transformation");
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
     viewport in the device frame [a square or rectangular region]). */

  /* begin by computing device coordinate ranges */
  switch (_plotter->display_model_type)
    {
    case (int)DISP_MODEL_NONE:
    default:
      /* Plotter has no device model, i.e., no knowledge of its output
	 device, i.e., no notion of a device frame at all.  (E.g., generic
	 and Metafile Plotters.)  So punt: pretend device frame is the same
	 as the NCD frame. */
	  device_x_left = device_y_bottom = 0.0;
	  device_x_right = device_y_top = 1.0;
	  break;
    case (int)DISP_MODEL_PHYSICAL:
      /* Plotter has a physical display, ranges in device coordinates of
	 the viewport are known (they're computed from the PAGESIZE
	 parameter when the Plotter is created, see ?_defplot.c).  E.g.,
	 for AI, Fig, HPGL, PCL, and PS Plotters. */
      {
	device_x_left = _plotter->xmin;
	device_x_right = _plotter->xmax;
	device_y_bottom = _plotter->ymin;
	device_y_top = _plotter->ymax;
      }
      break;
    case (int)DISP_MODEL_VIRTUAL:
      /* Plotter has a display, but its size isn't specified in physical
         units such as inches.  E.g., CGM, GIF, PNM, Tektronix, X, and X
         Drawable Plotters.  CGM Plotters are a curious hybrid: the
         PAGESIZE parameter is meaningful for them, at least insofar as
         viewport size goes, but we regard a CGM display as `virtual'
         because a CGM viewer or interpreter is free to ignore the
         requested viewport size.  */
      switch ((int)_plotter->display_coors_type)
	{
	case (int)DISP_DEVICE_COORS_REAL:
	default:
	  /* Real-coordinate virtual display device.  None of our Plotters
	     currently falls into this class.  So punt: treat NCD space as
	     device space. */
	  device_x_left = device_y_bottom = 0.0;
	  device_x_right = device_y_top = 1.0;
	  break;
	case (int)DISP_DEVICE_COORS_INTEGER_LIBXMI:
	case (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI:
	  /* Integer-coordinate virtual display device, whether using
	     libxmi-compatible scan conversion or not.  Of the Plotters
	     that have virtual displays (see above), GIF, PNM, X, and X
	     Drawable Plotters use libxmi-compatible scan conversion;
	     Tektronix Plotters and CGM Plotters do not.

	     In both cases, compute device coordinate ranges from imin,
	     imax, jmin, jmax, which are already available (see
	     ?_defplot.c; e.g., for Plotters with adjustable-size displays,
	     they are taken from the BITMAPSIZE parameter).  

	     The subtraction/addition of 0.5-ROUNDING_FUZZ is magic.  */
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
	  break;
	}
      break;
    }

  /* device coordinate ranges now known, so work out affine transformation
     #2, from the NCD frame to the device frame; take ROTATION parameter
     into account */

  rotation_s = (const char *)_get_plot_param (R___(_plotter) "ROTATION");
  if (rotation_s == NULL)
    rotation_s = (const char *)_get_default_plot_param ("ROTATION");
  if (strcmp (rotation_s, "90") == 0
      || strcmp (rotation_s, "yes") == 0) /* "yes" means "90" */
    rotation_angle = (int)ROT_90;
  else if (strcmp (rotation_s, "180") == 0)
    rotation_angle = (int)ROT_180;
  else if (strcmp (rotation_s, "270") == 0)
    rotation_angle = (int)ROT_270;
  else rotation_angle = (int)ROT_0; /* default, includes "no" */

  switch (rotation_angle)
    {
    case (int)ROT_0:
    default:
      /* NCD point (0,0) [lower left corner] gets mapped into this */
      t[4] = device_x_left;
      t[5] = device_y_bottom;
      /* NCD vector (1,0) gets mapped into this */
      t[0] = device_x_right - device_x_left;
      t[1] = 0.0;
      /* NCD vector (0,1) gets mapped into this */
      t[2] = 0.0;
      t[3] = device_y_top - device_y_bottom;
      break;
    case (int)ROT_90:
      /* NCD point (0,0) [lower left corner] gets mapped into this */
      t[4] = device_x_right;
      t[5] = device_y_bottom;
      /* NCD vector (1,0) gets mapped into this */
      t[0] = 0.0;
      t[1] = device_y_top - device_y_bottom;
      /* NCD vector (0,1) gets mapped into this */
      t[2] = device_x_left - device_x_right;
      t[3] = 0.0;
      break;
    case (int)ROT_180:
      /* NCD point (0,0) [lower left corner] gets mapped into this */
      t[4] = device_x_right;
      t[5] = device_y_top;
      /* NCD vector (1,0) gets mapped into this */
      t[0] = device_x_left - device_x_right;
      t[1] = 0.0;
      /* NCD vector (0,1) gets mapped into this */
      t[2] = 0.0;
      t[3] = device_y_bottom - device_y_top;
      break;
    case (int)ROT_270:
      /* NCD point (0,0) [lower left corner] gets mapped into this */
      t[4] = device_x_left;
      t[5] = device_y_top;
      /* NCD vector (1,0) gets mapped into this */
      t[0] = 0.0;
      t[1] = device_y_bottom - device_y_top;
      /* NCD vector (0,1) gets mapped into this */
      t[2] = device_x_right - device_x_left;
      t[3] = 0.0;
      break;
    }
  
  /* compute affine transformation from the user frame to the device frame
     as product of affine transformations #1 and #2 */
  _matrix_product (s, t, _plotter->drawstate->transform.m);

  /* does map preserve axis directions? */
  _plotter->drawstate->transform.axes_preserved = 
    (_plotter->drawstate->transform.m[1] == 0.0 
     && _plotter->drawstate->transform.m[2] == 0.0) ? true : false;

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
      = ((_plotter->flipped_y ? -1 : 1) * det >= 0) ? true : false;
  }
  
  /* Compute matrix norm of linear transformation appearing in the affine
     map from the user frame to the NCD frame. */

  /* This minimum singular value isn't really the norm.  But it's the
     nominal device-frame line width divided by the actual user-frame
     line-width (see g_linewidth.c), and that's what we need. */
  _matrix_sing_vals (s, &min_sing_val, &max_sing_val);
  norm = min_sing_val;

  /* First, compute default line width and font size.  Default values will
     be switched to, later, if the user calls linewidth() or fontsize()
     with out-of-bound arguments. */

  if (_plotter->display_coors_type == (int)DISP_DEVICE_COORS_INTEGER_LIBXMI)
    /* using libxmi or a compatible rendering algorithm; so set default
       line width to zero (interpreted as specifying a Bresenham line) */
    _plotter->drawstate->default_line_width = 0.0;
  else
    /* not using libxmi or a compatible rendering algorithm; so set default
       line width to a nonzero fraction of the display size */
    _plotter->drawstate->default_line_width 
      = DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE / norm;

  _plotter->drawstate->default_font_size
    = DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE / norm;

  /* We wish to maintain backwards compatibility with traditional libplot,
     where the user is _not_ required to make initial calls to linewidth()
     and fontsize(), but _is_ required to make a single initial call to
     space().  So when this function is called for the first time, we set
     the line width and font size to the just-computed default values, and
     compute the device-frame line width if we're keeping track of it.

     On subsequent invocations, we update the device-frame line width if
     we're keeping track of it.  But we never invoke fontsize() or
     retrieve_font() to recompute and set the device-frame font size.
     That's because on X and X Drawable Plotters, it wouldn't be wise: the
     new device-frame font size may be so small or large as to be
     unavailable on the X server, and the user may in fact be planning to
     invoke fontsize() manually to select a font of an appropriate size.

     Even if the user doesn't plan on doing that, it's OK not to invoke
     fontsize() or retrieve_font() here, since it'll be invoked before
     rendering any string (see g_alabel.c). */

  if (_plotter->space_invoked == false)
    /* first invocation of fspace2() on this page */
    {
      if (_plotter->display_model_type == DISP_MODEL_NONE)
	/* no device-frame line width; just set user-frame line width */
	_plotter->drawstate->line_width 
	  = _plotter->drawstate->default_line_width;
      else
	/* invoke flinewidth(), because if we have a display model,
	   flinewidth() may do some necessary things besides just set the
	   user-frame line width; e.g. compute a device-frame line width */
	_plotter->flinewidth (R___(_plotter) 
			      _plotter->drawstate->default_line_width);

      _plotter->drawstate->font_size = _plotter->drawstate->default_font_size;
      _plotter->space_invoked = true;
    }
  else
    /* a subsequent invocation of fspace2() */
    {
      if (_plotter->display_model_type != DISP_MODEL_NONE)
	/* invoke flinewidth() as above, e.g. to update device-frame line
           width */
	_plotter->flinewidth (R___(_plotter) 
			      _plotter->drawstate->line_width);
    }

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_fspace (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
#else
_g_fspace (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;)
     double x0, y0, x1, y1;
#endif
{
  return _plotter->fspace2 (R___(_plotter) x0, y0, x1, y0, x0, y1);
}
