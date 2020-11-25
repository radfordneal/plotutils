/* This file contains the space method, which is a standard part of the
   libplot library.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a square.
   The arguments to the space routine are the lower left and upper right
   vertices of a `window' (a drawing rectangle), in user coordinates.  This
   window, whose axes are aligned with the coordinate axes, will be mapped
   affinely onto the square on the display device.

   This file also contains the space2 routine, which is a GNU extension to
   libplot.  The arguments to the space2 method are the vertices of a of a
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the square on the
   display device. */

/* In this MetaPlotter version, we don't maintain an affine map from user
   coordinates to device coordinates (we have no idea what device
   coordinates will be, since the emitted graphics metafile will be
   displayed by an invocation of `plot').  We do however perform some
   computations to initialize the font size and line width, in terms of the
   display size, to default values.  The font size in terms of user
   coordinates needs to be kept track of by a MetaPlotter, so that the
   labelwidth() operation will work. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* forward references */
static bool _set_initial_font_size __P((double x0, double y0, double x1, double y1, double x2, double y2));

int
#ifdef _HAVE_PROTOS
_m_space (int x0, int y0, int x1, int y1)
#else
_m_space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("space: invalid operation");
      return -1;
    }

  if (!_set_initial_font_size ((double)x0, (double)y0, 
			       (double)x1, (double)y0, 
			       (double)x0, (double)y1))
    return -1;

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d\n", 
		 SPACE, x0, y0, x1, y1);
      else
	{
	  putc (SPACE, _plotter->outstream);
	  _emit_integer (x0);
	  _emit_integer (y0);
	  _emit_integer (x1);
	  _emit_integer (y1);
	}
    }      
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fspace (double x0, double y0, double x1, double y1)
#else
_m_fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fspace: invalid operation");
      return -1;
    }

  if (!_set_initial_font_size (x0, y0, x1, y0, x0, y1))
    return -1;

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g %g\n", 
		 SPACE, x0, y0, x1, y1);
      else
	{
	  putc (FSPACE, _plotter->outstream);
	  _emit_float (x0);
	  _emit_float (y0);
	  _emit_float (x1);
	  _emit_float (y1);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_space2 (int x0, int y0, int x1, int y1, int x2, int y2)
#else
_m_space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("space2: invalid operation");
      return -1;
    }

  if (!_set_initial_font_size ((double)x0, (double)y0, 
			       (double)x1, (double)y1, 
			       (double)x2, (double)y2))
    return -1;

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d %d %d\n", 
		 SPACE2, x0, y0, x1, y1, x2, y2);
      else
	{
	  putc (SPACE2, _plotter->outstream);
	  _emit_integer (x0);
	  _emit_integer (y0);
	  _emit_integer (x1);
	  _emit_integer (y1);
	  _emit_integer (x2);
	  _emit_integer (y2);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fspace2 (double x0, double y0, double x1, double y1, double x2, double y2)
#else
_m_fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fspace2: invalid operation");
      return -1;
    }

  if (!_set_initial_font_size (x0, y0, x1, y1, x2, y2))
    return -1;

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g %g %g %g\n", 
		 FSPACE2, x0, y0, x1, y1, x2, y2);
      else
	{
	  putc (FSPACE2, _plotter->outstream);
	  _emit_float (x0);
	  _emit_float (y0);
	  _emit_float (x1);
	  _emit_float (y1);
	  _emit_float (x2);
	  _emit_float (y2);
	}
    }
  
  return 0;
}

/* This is a stripped-down version of the _g_fspace2() method [see
   g_space.c].  All it does is set an initial value for the font size in
   the drawing state, which is needed if the user invokes the labelwidth()
   operation on a MetaPlotter object.  The value is further updated by the
   fontsize() operation.  (The line width is initialized similarly.  But
   neither it nor the other MetaPlotter attributes, which we carefully
   keep track of, affect anything that is written to the graphics
   metafile.) */
static bool
#ifdef _HAVE_PROTOS
_set_initial_font_size (double x0, double y0, double x1, double y1, double x2, double y2)
#else
_set_initial_font_size (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
#endif
{
  double s[6];
  double v0x, v0y, v1x, v1y, v2x, v2y;
  double cross;
  double norm;

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
      return false;
    }

  /* linear transformation */  
  s[0] = v2y / cross;
  s[1] = -v1y / cross;
  s[2] = -v2x / cross;
  s[3] = v1x / cross;

  /* translation */
  s[4] = - (v0x * v2y - v0y * v2x) / cross;
  s[5] = (v0x * v1y - v0y * v1x) / cross;
  
  /* Compute matrix norm of linear transformation appearing in the affine
     map from the user frame to the NCD frame. */
  norm = _matrix_norm (s);

  /* We want to maintain backwards compatibility with traditional libplot,
     where the user is not required to make initial calls to linewidth()
     and fontsize(), but is required to make a single initial call to
     space().  So when the user calls space() [or space2() or fspace() or
     fspace2()], we set the line width and font size to something reasonable.

     We also set the default line width and font size to something
     reasonable.  The default values stored there are used if the user
     calls linewidth() or fontsize() with out-of-bound arguments. */

  _plotter->drawstate->default_font_size
    = DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH / norm;
  _plotter->drawstate->font_size
    = DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH / norm;
  _plotter->drawstate->true_font_size
    = DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH / norm;

  _plotter->drawstate->default_line_width 
    = DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH / norm;
  _plotter->drawstate->line_width 
    = DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH / norm;

  return true;
}
