/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a marker, of specified size, at a
   specified location.  This marker can be one of a list of standard
   symbols, or a single character in the current font.

   The `size' argument is expressed in terms of user coordinates.  If a
   character is being plotted, it is the em size of the font.  If a symbol
   is being plotted, the maximum dimensions of the symbol will be
   approximately 5/8 times `size'. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* by convention, markers below have max dimensions 5/8 em = 0.625 em,
   e.g. the circle marker has 5/8 em as diameter.  SCALE_FACTOR is one-half
   of this quantity */
#define SCALE_FACTOR 0.3125

/* also by convention, line width used while drawing markers below is
   the following fraction of an em */
#define LINE_SCALE 0.03125

/* argument for filltype (), when we draw the half-filled marker symbols (this
   isn't exactly half, but it looks better than half) */
#define NOMINAL_HALF 0xa000

int
#ifdef _HAVE_PROTOS
_g_fmarker (double x, double y, int type, double size)
#else
_g_fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  char label_buf[2];

  if (!_plotter->open)
    {
      _plotter->error ("fmarker: invalid operation");
      return -1;
    }

  _plotter->fmove (x, y); /* on exit, will be at (x,y) */

  if (type < 0)			/* silently return if marker type < 0 */
    return 0;
  type %= 256;			/* silently compute marker type mod 256 */

  if (type > 31)
    {
      /* savestate requires too much overhead here, should fix */
      _plotter->savestate();
      _plotter->ffontsize (size);
      _plotter->textangle (0);
      label_buf[0] = (char)type;
      label_buf[1] = '\0';
      _plotter->alabel ('c', 'c', label_buf);
      _plotter->restorestate();
    }
  else
    {
      /* begin by saving the five relevant drawing attributes */
      char *old_line_mode, *old_cap_mode, *old_join_mode;
      int old_fill_level;
      double old_line_width;

      old_line_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->line_mode) + 1);
      old_cap_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->cap_mode) + 1);
      old_join_mode = (char *)_plot_xmalloc (strlen (_plotter->drawstate->join_mode) + 1);

      strcpy (old_line_mode, _plotter->drawstate->line_mode);
      strcpy (old_cap_mode, _plotter->drawstate->cap_mode);
      strcpy (old_join_mode, _plotter->drawstate->join_mode);
      old_line_width = _plotter->drawstate->line_width;
      old_fill_level = _plotter->drawstate->fill_level;

      /* attributes for drawing all our marker symbols */
      _plotter->linemod ("solid");
      _plotter->capmod ("butt");
      _plotter->joinmod ("miter");
      _plotter->flinewidth (LINE_SCALE * size);

      size *= SCALE_FACTOR;

      switch (type)
	/* N.B. 5-pointed star should be added some day */
	{
	case M_NONE:		/* no-op */
	default:
	  break;
	case M_DOT:		/* dot, GKS 1 */
	  _plotter->fpointrel (0.0, 0.0);
	  break;
	case M_PLUS:		/* plus, GKS 2 */
	  _plotter->fmoverel (-size, 0.0);
	  _plotter->fcontrel (2 * size, 0.0);
	  _plotter->fmoverel (-size, -size);
	  _plotter->fcontrel (0.0, 2 * size);
	  _plotter->fmoverel (0.0, -size);
	  break;
	case M_ASTERISK:	/* asterisk, GKS 3 */
	  {
	    double vert = 0.5 * size;
	    double hori = 0.5 * M_SQRT3 * size;
	    
	    _plotter->fmoverel (0.0, -size);
	    _plotter->fcontrel (0.0, 2 * size);
	    _plotter->fmoverel (0.0, -size);

	    _plotter->fcontrel (hori, vert);
	    _plotter->fmoverel (-hori, -vert);
	    _plotter->fcontrel (-hori, -vert);
	    _plotter->fmoverel (hori, vert);
	    _plotter->fcontrel (hori, -vert);
	    _plotter->fmoverel (-hori, vert);
	    _plotter->fcontrel (-hori, vert);
	    _plotter->fmoverel (hori, -vert);
	  }
	  break;
	case M_CIRCLE:		/* circle, GKS 4 */
	  _plotter->filltype (0);
	  _plotter->fcirclerel (0.0, 0.0, size);
	  break;
	case M_CROSS:		/* cross, GKS 5 */
	  _plotter->fmoverel (-size, -size);
	  _plotter->fcontrel (2 * size, 2 * size);
	  _plotter->fmoverel (0.0, - 2 * size);
	  _plotter->fcontrel (-2 * size, 2 * size);
	  _plotter->fmoverel (size, -size);
	  break;
	case M_STAR:			/* star */
	  _plotter->fmoverel (-size, 0.0);
	  _plotter->fcontrel (2 * size, 0.0);
	  _plotter->fmoverel (-size, -size);
	  _plotter->fcontrel (0.0, 2 * size);
	  _plotter->fmoverel (0.0, -size);
	  _plotter->fcontrel (size, size);
	  _plotter->fmoverel (-size, -size);
	  _plotter->fcontrel (size, -size);
	  _plotter->fmoverel (-size, size);
	  _plotter->fcontrel (-size, size);
	  _plotter->fmoverel (size, -size);
	  _plotter->fcontrel (-size, -size);
	  _plotter->fmoverel (size, size);
	  break;
	case M_SQUARE:	/* square */
	  _plotter->filltype (0);
	  _plotter->fboxrel (-size, -size, size, size);
	  _plotter->fmoverel (-size, -size);
	  break;
	case M_DIAMOND:	/* diamond */
	  _plotter->filltype (0);
	  _plotter->fmoverel (size, 0.0);
	  _plotter->fcontrel (-size, size);
	  _plotter->fcontrel (-size, -size);
	  _plotter->fcontrel (size, -size);
	  _plotter->fcontrel (size, size);
	  _plotter->fmoverel (-size, 0.0);
	  break;
	case M_TRIANGLE:	/* triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (0);
	    _plotter->fmoverel (0.0, size);
	    _plotter->fcontrel (halfwidth, -1.5 * size);
	    _plotter->fcontrel (-2 * halfwidth, 0.0);
	    _plotter->fcontrel (halfwidth, 1.5 * size);
	    _plotter->fmoverel (0.0, -size);
	  }	  
	  break;
	case M_INVERTED_TRIANGLE: /* triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (0);
	    _plotter->fmoverel (0.0, -size);
	    _plotter->fcontrel (halfwidth, 1.5 * size);
	    _plotter->fcontrel (-2 * halfwidth, 0.0);
	    _plotter->fcontrel (halfwidth, -1.5 * size);
	    _plotter->fmoverel (0.0, size);
	  }	  
	  break;
	case M_FILLED_SQUARE:	/* filled square */
	  _plotter->filltype (1);
	  _plotter->fboxrel (-size, -size, size, size);
	  _plotter->fmoverel (-size, -size);
	  break;
	case M_FILLED_DIAMOND:	/* filled diamond */
	  _plotter->filltype (1);
	  _plotter->fmoverel (0.0, -size);
	  _plotter->fcontrel (size, size);
	  _plotter->fcontrel (-size, size);
	  _plotter->fcontrel (-size, -size);
	  _plotter->fcontrel (size, -size);
	  _plotter->fmoverel (0.0, size);
	  break;
	case M_FILLED_TRIANGLE: /* filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (1);
	    _plotter->fmoverel (0.0, size);
	    _plotter->fcontrel (halfwidth, -1.5 * size);
	    _plotter->fcontrel (-2 * halfwidth, 0.0);
	    _plotter->fcontrel (halfwidth, 1.5 * size);
	    _plotter->fmoverel (0.0, -size);
	  }	  
	  break;
	case M_FILLED_INVERTED_TRIANGLE: /* filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (1);
	    _plotter->fmoverel (0.0, -size);
	    _plotter->fcontrel (halfwidth, 1.5 * size);
	    _plotter->fcontrel (-2 * halfwidth, 0.0);
	    _plotter->fcontrel (halfwidth, -1.5 * size);
	    _plotter->fmoverel (0.0, size);
	  }	  
	  break;
	case M_FILLED_CIRCLE:	/* filled circle */
	  _plotter->filltype (1);
	  fcirclerel (0.0, 0.0, size);
	  break;
	case M_STARBURST:	/* starburst */
	  _plotter->fmoverel (-0.5 * size, 0.0);
	  _plotter->fcontrel (-0.5 * size, 0.0);
	  _plotter->fmoverel (0.0, -size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->fmoverel (0.5 * size, 0.0);
	  _plotter->fcontrel (0.0, -0.5 * size);
	  _plotter->fmoverel (size, 0.0);
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fmoverel (0.0, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.0);
	  _plotter->fmoverel (0.0, size);
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fmoverel (-0.5 * size, 0.0);
	  _plotter->fcontrel (0.0, 0.5 * size);
	  _plotter->fmoverel (-size, 0.0);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fmoverel (0.5 * size, -0.5 * size);
	  break;
	case M_FANCY_PLUS:	/* ornate plus */
	  _plotter->fmoverel (-size, 0.0);
	  _plotter->fcontrel (2 * size, 0.0);
	  _plotter->fmoverel (-size, -size);
	  _plotter->fcontrel (0.0, 2 * size);
	  _plotter->fmoverel (0.5 * size, 0.0);
	  _plotter->fcontrel (-size, 0.0);
	  _plotter->fmoverel (-0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.0, -size);
	  _plotter->fmoverel (0.5 * size, -0.5 * size);
	  _plotter->fcontrel (size, 0.0);
	  _plotter->fmoverel (0.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.0, size);
	  _plotter->fmoverel (-size, 0.0);
	  break;
	case M_FANCY_CROSS:	/* ornate cross */
	  _plotter->fmoverel (-size, -size);
	  _plotter->fcontrel (2 * size, 2 * size);
	  _plotter->fmoverel (0.0, -2 * size);
	  _plotter->fcontrel (-2 * size, 2 * size);
	  _plotter->fmoverel (2 * size, -0.5 * size);
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fmoverel (-size, 0.0);
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fmoverel (0.0, -size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fmoverel (size, 0.0);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->fmoverel (-size, 0.5 * size);
	  break;
	case M_FANCY_SQUARE:	/* ornate square */
	  _plotter->filltype (0);
	  fboxrel (-0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->fmoverel (-1.5 * size, -1.5 * size);	  
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fmoverel (1.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fmoverel (-1.5 * size, 1.5 * size);	  
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fmoverel (1.5 * size, -1.5 * size);	  
	  break;
	case M_FANCY_DIAMOND:	/* diamond */
	  _plotter->filltype (0);
	  _plotter->fmoverel (0.5 * size, 0.0);
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->endpath();
	  _plotter->fcontrel (0.5 * size, 0.0);
	  _plotter->fmoverel (-size, 0.5 * size);
	  _plotter->fcontrel (0.0, 0.5 * size);
	  _plotter->fmoverel (-0.5 * size, -size);
	  _plotter->fcontrel (-0.5 * size, 0.0);
	  _plotter->fmoverel (size, -0.5 * size);
	  _plotter->fcontrel (0.0, -0.5 * size);
	  _plotter->fmoverel (0.0, size);
	  break;
	case M_FILLED_FANCY_SQUARE:	/* filled ornate square */
	  _plotter->filltype (1);
	  fboxrel (-0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->fmoverel (-1.5 * size, -1.5 * size);	  
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fmoverel (1.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fmoverel (-1.5 * size, 1.5 * size);	  
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fmoverel (1.5 * size, -1.5 * size);	  
	  break;
	case M_FILLED_FANCY_DIAMOND: /* filled ornate diamond */
	  _plotter->filltype (1);
	  _plotter->fmoverel (0.5 * size, 0.0);
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->endpath();
	  _plotter->fcontrel (0.5 * size, 0.0);
	  _plotter->fmoverel (-size, 0.5 * size);
	  _plotter->fcontrel (0.0, 0.5 * size);
	  _plotter->fmoverel (-0.5 * size, -size);
	  _plotter->fcontrel (-0.5 * size, 0.0);
	  _plotter->fmoverel (size, -0.5 * size);
	  _plotter->fcontrel (0.0, -0.5 * size);
	  _plotter->fmoverel (0.0, size);
	  break;
	case M_HALF_FILLED_SQUARE:	/* half_filled square */
	  _plotter->filltype (NOMINAL_HALF);
	  fboxrel (-size, -size, size, size);
	  _plotter->fmoverel (-size, -size);
	  break;
	case M_HALF_FILLED_DIAMOND:	/* half_filled diamond */
	  _plotter->filltype (NOMINAL_HALF);
	  _plotter->fmoverel (0.0, -size);
	  _plotter->fcontrel (size, size);
	  _plotter->fcontrel (-size, size);
	  _plotter->fcontrel (-size, -size);
	  _plotter->fcontrel (size, -size);
	  _plotter->fmoverel (0.0, size);
	  break;
	case M_HALF_FILLED_TRIANGLE: /* half_filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (NOMINAL_HALF);
	    _plotter->fmoverel (0.0, size);
	    _plotter->fcontrel (halfwidth, -1.5 * size);
	    _plotter->fcontrel (-2 * halfwidth, 0.0);
	    _plotter->fcontrel (halfwidth, 1.5 * size);
	    _plotter->fmoverel (0.0, -size);
	  }	  
	  break;
	case M_HALF_FILLED_INVERTED_TRIANGLE: /* half_filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _plotter->filltype (NOMINAL_HALF);
	    _plotter->fmoverel (0.0, -size);
	    _plotter->fcontrel (halfwidth, 1.5 * size);
	    _plotter->fcontrel (-2 * halfwidth, 0.0);
	    _plotter->fcontrel (halfwidth, -1.5 * size);
	    _plotter->fmoverel (0.0, size);
	  }	  
	  break;
	case M_HALF_FILLED_CIRCLE:	/* half_filled circle */
	  _plotter->filltype (NOMINAL_HALF);
	  fcirclerel (0.0, 0.0, size);
	  break;
	case M_HALF_FILLED_FANCY_SQUARE:  /* half-filled ornate square */
	  _plotter->filltype (NOMINAL_HALF);
	  fboxrel (-0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->fmoverel (-1.5 * size, -1.5 * size);	  
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fmoverel (1.5 * size, 0.5 * size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fmoverel (-1.5 * size, 1.5 * size);	  
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fmoverel (1.5 * size, -1.5 * size);	  
	  break;
	case M_HALF_FILLED_FANCY_DIAMOND: /* half-filled ornate diamond */
	  _plotter->filltype (NOMINAL_HALF);
	  _plotter->fmoverel (0.5 * size, 0.0);
	  _plotter->fcontrel (-0.5 * size, 0.5 * size);
	  _plotter->fcontrel (-0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.5 * size, -0.5 * size);
	  _plotter->fcontrel (0.5 * size, 0.5 * size);
	  _plotter->endpath();
	  _plotter->fcontrel (0.5 * size, 0.0);
	  _plotter->fmoverel (-size, 0.5 * size);
	  _plotter->fcontrel (0.0, 0.5 * size);
	  _plotter->fmoverel (-0.5 * size, -size);
	  _plotter->fcontrel (-0.5 * size, 0.0);
	  _plotter->fmoverel (size, -0.5 * size);
	  _plotter->fcontrel (0.0, -0.5 * size);
	  _plotter->fmoverel (0.0, size);
	  break;
	case M_OCTAGON:		/* octagon */
	  _plotter->filltype (0);
	  _plotter->fmoverel (-2.0 * size, size);
	  _plotter->fcontrel (0.0, -2.0 * size);
	  _plotter->fcontrel (size, -size);
	  _plotter->fcontrel (2.0 * size, 0.0);
	  _plotter->fcontrel (size, size);
	  _plotter->fcontrel (0.0, 2.0 * size);
	  _plotter->fcontrel (-size, size);
	  _plotter->fcontrel (-2.0 * size, 0.0);
	  _plotter->fcontrel (-size, -size);
	  _plotter->fmoverel (2.0 * size, -size);
	  break;
	case M_FILLED_OCTAGON:	/* filled octagon */
	  _plotter->filltype (1);
	  _plotter->fmoverel (-2.0 * size, size);
	  _plotter->fcontrel (0.0, -2.0 * size);
	  _plotter->fcontrel (size, -size);
	  _plotter->fcontrel (2.0 * size, 0.0);
	  _plotter->fcontrel (size, size);
	  _plotter->fcontrel (0.0, 2.0 * size);
	  _plotter->fcontrel (-size, size);
	  _plotter->fcontrel (-2.0 * size, 0.0);
	  _plotter->fcontrel (-size, -size);
	  _plotter->fmoverel (2.0 * size, -size);
	  break;
	}

      /* restore the original values of the five relevant drawing attributes,
	 and free storage */
      _plotter->linemod (old_line_mode);
      _plotter->capmod (old_cap_mode);
      _plotter->joinmod (old_join_mode);
      _plotter->flinewidth (old_line_width);
      _plotter->filltype (old_fill_level);
      
      free (old_line_mode);
      free (old_cap_mode);
      free (old_join_mode);
    }
  
  return 0;
}
