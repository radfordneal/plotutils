/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a marker, of specified size, at a
   specified location.  This marker may be one of a list of standard
   symbols, or a single character in the current font.

   The `size' argument is expressed in terms of user coordinates.  If the
   marker is a character, it is the font size (i.e., the em size of the
   font).  If the marker is a symbol, the maximum dimension of the symbol
   will be a fixed fraction of `size'. */

/* This is a generic version.  Currently, it is overridden only by Metafile
   and CGM Plotters. */

#include "sys-defines.h"
#include "extern.h"

/* The maximum dimension of most markers, e.g. the diameter of the circle
   marker (marker #4).  Expressed as a fraction of the `size' argument. */
#define MAXIMUM_MARKER_DIMENSION (5.0/8.0)

/* Line width used while drawing marker symbols.  Expressed as a fraction
   of the `size' argument. */
#define LINE_SCALE (0.05 * MAXIMUM_MARKER_DIMENSION)

/* The diameter of a `dot' (marker #1), as a fraction of the maximum marker
   dimension (see above).  This applies only if the Plotter has a physical
   display, i.e. the PAGESIZE parameter is meaningful for it, or if the
   Plotter has a virtual display that uses real coordinates.  Currently,
   none of our Plotters falls into the 2nd category.

   If the Plotter falls into neither of these two categories, we assume
   that fpoint() draws a single pixel, or something like it, and we use
   fpoint() to draw marker #1.  E.g., we do so on Tektronix, GIF, PNM, X,
   and X Drawable displays.  For them, RELATIVE_DOT_SIZE is ignored. */

#define RELATIVE_DOT_SIZE 0.15

/* Argument for filltype(), when we draw the half-filled marker symbols.
   (This isn't exactly half, but it looks better than half.) */
#define NOMINAL_HALF 0xa000

int
#ifdef _HAVE_PROTOS
_g_fmarker (R___(Plotter *_plotter) double x, double y, int type, double size)
#else
_g_fmarker (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;)
     double x, y;
     int type;
     double size;
#endif
{
  char label_buf[2];
  double x_dev, y_dev, delta_x_dev, delta_y_dev;
  double delta_x_user = 0.0, delta_y_user = 0.0;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmarker: invalid operation");
      return -1;
    }

  _plotter->fmove (R___(_plotter) x, y); /* on exit, will be at (x,y) */

  if (type < 0)			/* silently return if marker type < 0 */
    return 0;
  type %= 256;			/* silently compute marker type mod 256 */

  if (_plotter->display_coors_type != (int)DISP_DEVICE_COORS_REAL)
    /* move temporarily to nearest point with integer device coordinates,
       so that symbol will be symmetrically positioned */
    {
      x_dev = XD(x, y);
      y_dev = YD(x, y);
      delta_x_dev = IROUND(x_dev) - x_dev;
      delta_y_dev = IROUND(y_dev) - y_dev;
      delta_x_user = XUV(delta_x_dev, delta_y_dev);
      delta_y_user = YUV(delta_x_dev, delta_y_dev);

      _plotter->drawstate->pos.x += delta_x_user;
      _plotter->drawstate->pos.y += delta_y_user;
    }

  if (type > 31)
    {
      /* savestate requires too much overhead here, should fix */
      _plotter->savestate (S___(_plotter));
      _plotter->ffontsize (R___(_plotter) size);
      _plotter->textangle (R___(_plotter) 0);
      label_buf[0] = (char)type;
      label_buf[1] = '\0';
      _plotter->alabel (R___(_plotter) 'c', 'c', label_buf);
      _plotter->restorestate (S___(_plotter));
    }
  else
    {
      /* begin by saving the relevant drawing attributes */
      char *old_line_mode, *old_cap_mode, *old_join_mode;
      int old_fill_type;
      double old_line_width;
      bool old_dash_array_in_effect;

      old_line_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->line_mode) + 1);
      old_cap_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->cap_mode) + 1);
      old_join_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->join_mode) + 1);

      strcpy (old_line_mode, _plotter->drawstate->line_mode);
      strcpy (old_cap_mode, _plotter->drawstate->cap_mode);
      strcpy (old_join_mode, _plotter->drawstate->join_mode);

      old_dash_array_in_effect = _plotter->drawstate->dash_array_in_effect;
      old_line_width = _plotter->drawstate->line_width;
      old_fill_type = _plotter->drawstate->fill_type;

      /* attributes for drawing all our marker symbols */
      _plotter->linemod (R___(_plotter) "solid");
      _plotter->capmod (R___(_plotter) "butt");
      _plotter->joinmod (R___(_plotter) "miter");
      _plotter->flinewidth (R___(_plotter) LINE_SCALE * size);

      /* beautification kludge: if display is a bitmap device using libxmi
	 or a compatible scan-conversion scheme, and line thickness is one
	 pixel or less, but not zero, then change it to zero thickess
	 (which will be interpreted as specifying a Bresenham line). */
      if (_plotter->display_coors_type == (int)DISP_DEVICE_COORS_INTEGER_LIBXMI
	  && _plotter->drawstate->quantized_device_line_width == 1)
	_plotter->flinewidth (R___(_plotter) 0.0);	

      size *= (0.5 * MAXIMUM_MARKER_DIMENSION);

      switch (type)
	/* N.B. 5-pointed star should be added some day */
	{
	case (int)M_NONE:		/* no-op */
	default:
	  break;
	case (int)M_DOT:		/* dot, GKS 1 */
	  _plotter->filltype (R___(_plotter) 1);
	  if (_plotter->display_model_type != (int)DISP_MODEL_VIRTUAL
	      || _plotter->display_coors_type == (int)DISP_DEVICE_COORS_REAL)
	  _plotter->fcirclerel (R___(_plotter) 0.0, 0.0, 
				RELATIVE_DOT_SIZE * size);
	  else			/* see comment above */
	    _plotter->fpointrel (R___(_plotter) 0.0, 0.0);
	  break;
	case (int)M_PLUS:		/* plus, GKS 2 */
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 2 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) 0.0, 2 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  break;
	case (int)M_ASTERISK:	/* asterisk, GKS 3 */
	  {
	    double vert = 0.5 * size;
	    double hori = 0.5 * M_SQRT3 * size;
	    
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	    _plotter->fcontrel (R___(_plotter) 0.0, 2 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);

	    _plotter->fcontrel (R___(_plotter) hori, vert);
	    _plotter->fmoverel (R___(_plotter) -hori, -vert);
	    _plotter->fcontrel (R___(_plotter) -hori, -vert);
	    _plotter->fmoverel (R___(_plotter) hori, vert);
	    _plotter->fcontrel (R___(_plotter) hori, -vert);
	    _plotter->fmoverel (R___(_plotter) -hori, vert);
	    _plotter->fcontrel (R___(_plotter) -hori, vert);
	    _plotter->fmoverel (R___(_plotter) hori, -vert);
	  }
	  break;
	case (int)M_CIRCLE:		/* circle, GKS 4 */
	  _plotter->filltype (R___(_plotter) 0);
	  _plotter->fcirclerel (R___(_plotter) 0.0, 0.0, size);
	  break;
	case (int)M_CROSS:		/* cross, GKS 5 */
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) 2 * size, 2 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, - 2 * size);
	  _plotter->fcontrel (R___(_plotter) -2 * size, 2 * size);
	  _plotter->fmoverel (R___(_plotter) size, -size);
	  break;
	case (int)M_STAR:			/* star */
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 2 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) 0.0, 2 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) size, size);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) size, -size);
	  _plotter->fmoverel (R___(_plotter) -size, size);
	  _plotter->fcontrel (R___(_plotter) -size, size);
	  _plotter->fmoverel (R___(_plotter) size, -size);
	  _plotter->fcontrel (R___(_plotter) -size, -size);
	  _plotter->fmoverel (R___(_plotter) size, size);
	  break;
	case (int)M_SQUARE:	/* square */
	  _plotter->filltype (R___(_plotter) 0);
	  _plotter->fboxrel (R___(_plotter) -size, -size, size, size);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  break;
	case (int)M_DIAMOND:	/* diamond */
	  _plotter->filltype (R___(_plotter) 0);
	  _plotter->fmoverel (R___(_plotter) size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -size, size);
	  _plotter->fcontrel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) size, -size);
	  _plotter->fcontrel (R___(_plotter) size, size);
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  break;
	case (int)M_TRIANGLE:	/* triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (R___(_plotter) 0);
	    _plotter->fmoverel (R___(_plotter) 0.0, size);
	    _plotter->fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _plotter->fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _plotter->fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  }	  
	  break;
	case (int)M_INVERTED_TRIANGLE: /* triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (R___(_plotter) 0);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	    _plotter->fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _plotter->fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _plotter->fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, size);
	  }	  
	  break;
	case (int)M_FILLED_SQUARE:	/* filled square */
	  _plotter->filltype (R___(_plotter) 1);
	  _plotter->fboxrel (R___(_plotter) -size, -size, size, size);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  break;
	case (int)M_FILLED_DIAMOND:	/* filled diamond */
	  _plotter->filltype (R___(_plotter) 1);
	  _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) size, size);
	  _plotter->fcontrel (R___(_plotter) -size, size);
	  _plotter->fcontrel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) size, -size);
	  _plotter->fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_FILLED_TRIANGLE: /* filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (R___(_plotter) 1);
	    _plotter->fmoverel (R___(_plotter) 0.0, size);
	    _plotter->fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _plotter->fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _plotter->fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  }	  
	  break;
	case (int)M_FILLED_INVERTED_TRIANGLE: /* filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (R___(_plotter) 1);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	    _plotter->fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _plotter->fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _plotter->fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, size);
	  }	  
	  break;
	case (int)M_FILLED_CIRCLE:	/* filled circle */
	  _plotter->filltype (R___(_plotter) 1);
	  _plotter->fcirclerel (R___(_plotter) 0.0, 0.0, size);
	  break;
	case (int)M_STARBURST:	/* starburst */
	  _plotter->fmoverel (R___(_plotter) -0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) 0.0, size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, -0.5 * size);
	  break;
	case (int)M_FANCY_PLUS:	/* ornate plus */
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 2 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) 0.0, 2 * size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, -size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) size, 0.0);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, size);
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  break;
	case (int)M_FANCY_CROSS:	/* ornate cross */
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) 2 * size, 2 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, -2 * size);
	  _plotter->fcontrel (R___(_plotter) -2 * size, 2 * size);
	  _plotter->fmoverel (R___(_plotter) 2 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -size, 0.5 * size);
	  break;
	case (int)M_FANCY_SQUARE:	/* ornate square */
	  _plotter->filltype (R___(_plotter) 0);
	  _plotter->fboxrel (R___(_plotter) -0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -1.5 * size, -1.5 * size);	  
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 1.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -1.5 * size, 1.5 * size);	  
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 1.5 * size, -1.5 * size);	  
	  break;
	case (int)M_FANCY_DIAMOND:	/* diamond */
	  _plotter->filltype (R___(_plotter) 0);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->endpath (S___(_plotter));
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -0.5 * size, -size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_FILLED_FANCY_SQUARE:	/* filled ornate square */
	  _plotter->filltype (R___(_plotter) 1);
	  _plotter->fboxrel (R___(_plotter) -0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -1.5 * size, -1.5 * size);	  
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 1.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -1.5 * size, 1.5 * size);	  
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 1.5 * size, -1.5 * size);	  
	  break;
	case (int)M_FILLED_FANCY_DIAMOND: /* filled ornate diamond */
	  _plotter->filltype (R___(_plotter) 1);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->endpath (S___(_plotter));
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -0.5 * size, -size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_HALF_FILLED_SQUARE:	/* half_filled square */
	  _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	  _plotter->fboxrel (R___(_plotter) -size, -size, size, size);
	  _plotter->fmoverel (R___(_plotter) -size, -size);
	  break;
	case (int)M_HALF_FILLED_DIAMOND:	/* half_filled diamond */
	  _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	  _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) size, size);
	  _plotter->fcontrel (R___(_plotter) -size, size);
	  _plotter->fcontrel (R___(_plotter) -size, -size);
	  _plotter->fcontrel (R___(_plotter) size, -size);
	  _plotter->fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_HALF_FILLED_TRIANGLE: /* half_filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	    _plotter->fmoverel (R___(_plotter) 0.0, size);
	    _plotter->fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _plotter->fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _plotter->fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	  }	  
	  break;
	case (int)M_HALF_FILLED_INVERTED_TRIANGLE: /* half_filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	    _plotter->fmoverel (R___(_plotter) 0.0, -size);
	    _plotter->fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _plotter->fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _plotter->fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _plotter->fmoverel (R___(_plotter) 0.0, size);
	  }	  
	  break;
	case (int)M_HALF_FILLED_CIRCLE:	/* half_filled circle */
	  _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	  _plotter->fcirclerel (R___(_plotter) 0.0, 0.0, size);
	  break;
	case (int)M_HALF_FILLED_FANCY_SQUARE:  /* half-filled ornate square */
	  _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	  _plotter->fboxrel (R___(_plotter) -0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -1.5 * size, -1.5 * size);	  
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 1.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -1.5 * size, 1.5 * size);	  
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 1.5 * size, -1.5 * size);	  
	  break;
	case (int)M_HALF_FILLED_FANCY_DIAMOND: /* half-filled ornate diamond */
	  _plotter->filltype (R___(_plotter) NOMINAL_HALF);
	  _plotter->fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->endpath (S___(_plotter));
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) -size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _plotter->fmoverel (R___(_plotter) -0.5 * size, -size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _plotter->fmoverel (R___(_plotter) size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_OCTAGON:		/* octagon */
	  _plotter->filltype (R___(_plotter) 0);
	  _plotter->fmoverel (R___(_plotter) -size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) size, -0.5 * size);
	  break;
	case (int)M_FILLED_OCTAGON:	/* filled octagon */
	  _plotter->filltype (R___(_plotter) 1);
	  _plotter->fmoverel (R___(_plotter) -size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, -size);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _plotter->fcontrel (R___(_plotter) size, 0.0);
	  _plotter->fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) 0.0, size);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _plotter->fcontrel (R___(_plotter) -size, 0.0);
	  _plotter->fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _plotter->fmoverel (R___(_plotter) size, -0.5 * size);
	  break;
	}

      /* restore the original values of the relevant drawing attributes,
	 and free storage */
      _plotter->linemod (R___(_plotter) old_line_mode);
      _plotter->capmod (R___(_plotter) old_cap_mode);
      _plotter->joinmod (R___(_plotter) old_join_mode);
      _plotter->flinewidth (R___(_plotter) old_line_width);
      _plotter->filltype (R___(_plotter) old_fill_type);
      _plotter->drawstate->dash_array_in_effect = old_dash_array_in_effect;
      
      free (old_line_mode);
      free (old_cap_mode);
      free (old_join_mode);
    }
  
  if (_plotter->display_coors_type != (int)DISP_DEVICE_COORS_REAL)
    /* undo the small repositioning (see above) */
    {
      _plotter->drawstate->pos.x -= delta_x_user;
      _plotter->drawstate->pos.y -= delta_y_user;
    }

  return 0;
}
