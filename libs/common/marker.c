/* This file contains the marker routine, which is a GNU extension to
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

/* argument for fill(), when we draw the half-filled marker symbols (this
   isn't exactly half, but it looks better than half) */
#define NOMINAL_HALF 0xa000

int
fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
{
  char label_buf[2];

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fmarker() called when graphics device not open\n");
      return -1;
    }

  fmove (x, y);			/* on exit, will be at (x,y) */

  if (type < 0)			/* silently return if marker type < 0 */
    return 0;
  type %= 256;			/* silently compute marker type mod 256 */

  if (type > 31)
    {
      /* savestate requires too much overhead here, should fix */
      savestate();
      ffontsize (size);
      textangle (0);
      label_buf[0] = (char)type;
      label_buf[1] = '\0';
      alabel ('c', 'c', label_buf);
      restorestate();
    }
  else
    {
      /* begin by saving the five relevant drawing attributes */
      char *old_line_mode, *old_cap_mode, *old_join_mode;
      int old_fill_level;
      double old_line_width;

      old_line_mode = (char *)_plot_xmalloc (strlen (_drawstate->line_mode) + 1);
      old_cap_mode = (char *)_plot_xmalloc (strlen (_drawstate->cap_mode) + 1);
      old_join_mode = (char *)_plot_xmalloc (strlen (_drawstate->join_mode) + 1);

      strcpy (old_line_mode, _drawstate->line_mode);
      strcpy (old_cap_mode, _drawstate->cap_mode);
      strcpy (old_join_mode, _drawstate->join_mode);
      old_line_width = _drawstate->line_width;
      old_fill_level = _drawstate->fill_level;

      /* attributes for drawing all our marker symbols */
      linemod ("solid");
      capmod ("butt");
      joinmod ("miter");
      flinewidth (_default_drawstate.line_width);

      size *= SCALE_FACTOR;

      switch (type)
	/* N.B. 5-pointed star should be added some day */
	{
	case M_NONE:		/* no-op */
	default:
	  break;
	case M_DOT:		/* dot, GKS 1 */
	  fpointrel (0.0, 0.0);
	  break;
	case M_PLUS:		/* plus, GKS 2 */
	  fmoverel (-size, 0.0);
	  fcontrel (2 * size, 0.0);
	  fmoverel (-size, -size);
	  fcontrel (0.0, 2 * size);
	  fmoverel (0.0, -size);
	  break;
	case M_ASTERISK:	/* asterisk, GKS 3 */
	  {
	    double vert = 0.5 * size;
	    double hori = 0.5 * M_SQRT3 * size;
	    
	    fmoverel (0.0, -size);
	    fcontrel (0.0, 2 * size);
	    fmoverel (0.0, -size);

	    fcontrel (hori, vert);
	    fmoverel (-hori, -vert);
	    fcontrel (-hori, -vert);
	    fmoverel (hori, vert);
	    fcontrel (hori, -vert);
	    fmoverel (-hori, vert);
	    fcontrel (-hori, vert);
	    fmoverel (hori, -vert);
	  }
	  break;
	case M_CIRCLE:		/* circle, GKS 4 */
	  fill(0);
	  fcirclerel (0.0, 0.0, size);
	  break;
	case M_CROSS:		/* cross, GKS 5 */
	  fmoverel (-size, -size);
	  fcontrel (2 * size, 2 * size);
	  fmoverel (0.0, - 2 * size);
	  fcontrel (-2 * size, 2 * size);
	  fmoverel (size, -size);
	  break;
	case M_STAR:			/* star */
	  fmoverel (-size, 0.0);
	  fcontrel (2 * size, 0.0);
	  fmoverel (-size, -size);
	  fcontrel (0.0, 2 * size);
	  fmoverel (0.0, -size);
	  fcontrel (size, size);
	  fmoverel (-size, -size);
	  fcontrel (size, -size);
	  fmoverel (-size, size);
	  fcontrel (-size, size);
	  fmoverel (size, -size);
	  fcontrel (-size, -size);
	  fmoverel (size, size);
	  break;
	case M_SQUARE:	/* square */
	  fill(0);
	  fboxrel (-size, -size, size, size);
	  fmoverel (-size, -size);
	  break;
	case M_DIAMOND:	/* diamond */
	  fill(0);
	  fmoverel (size, 0.0);
	  fcontrel (-size, size);
	  fcontrel (-size, -size);
	  fcontrel (size, -size);
	  fcontrel (size, size);
	  fmoverel (-size, 0.0);
	  break;
	case M_TRIANGLE:	/* triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    fill(0);
	    fmoverel (0.0, size);
	    fcontrel (halfwidth, -1.5 * size);
	    fcontrel (-2 * halfwidth, 0.0);
	    fcontrel (halfwidth, 1.5 * size);
	    fmoverel (0.0, -size);
	  }	  
	  break;
	case M_INVERTED_TRIANGLE: /* triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    fill(0);
	    fmoverel (0.0, -size);
	    fcontrel (halfwidth, 1.5 * size);
	    fcontrel (-2 * halfwidth, 0.0);
	    fcontrel (halfwidth, -1.5 * size);
	    fmoverel (0.0, size);
	  }	  
	  break;
	case M_FILLED_SQUARE:	/* filled square */
	  fill(1);
	  fboxrel (-size, -size, size, size);
	  fmoverel (-size, -size);
	  break;
	case M_FILLED_DIAMOND:	/* filled diamond */
	  fill(1);
	  fmoverel (0.0, -size);
	  fcontrel (size, size);
	  fcontrel (-size, size);
	  fcontrel (-size, -size);
	  fcontrel (size, -size);
	  fmoverel (0.0, size);
	  break;
	case M_FILLED_TRIANGLE: /* filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    fill(1);
	    fmoverel (0.0, size);
	    fcontrel (halfwidth, -1.5 * size);
	    fcontrel (-2 * halfwidth, 0.0);
	    fcontrel (halfwidth, 1.5 * size);
	    fmoverel (0.0, -size);
	  }	  
	  break;
	case M_FILLED_INVERTED_TRIANGLE: /* filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    fill(1);
	    fmoverel (0.0, -size);
	    fcontrel (halfwidth, 1.5 * size);
	    fcontrel (-2 * halfwidth, 0.0);
	    fcontrel (halfwidth, -1.5 * size);
	    fmoverel (0.0, size);
	  }	  
	  break;
	case M_FILLED_CIRCLE:	/* filled circle */
	  fill(1);
	  fcirclerel (0.0, 0.0, size);
	  break;
	case M_STARBURST:	/* starburst */
	  fmoverel (-0.5 * size, 0.0);
	  fcontrel (-0.5 * size, 0.0);
	  fmoverel (0.0, -size);
	  fcontrel (0.5 * size, 0.5 * size);
	  fmoverel (0.5 * size, 0.0);
	  fcontrel (0.0, -0.5 * size);
	  fmoverel (size, 0.0);
	  fcontrel (-0.5 * size, 0.5 * size);
	  fmoverel (0.0, 0.5 * size);
	  fcontrel (0.5 * size, 0.0);
	  fmoverel (0.0, size);
	  fcontrel (-0.5 * size, -0.5 * size);
	  fmoverel (-0.5 * size, 0.0);
	  fcontrel (0.0, 0.5 * size);
	  fmoverel (-size, 0.0);
	  fcontrel (0.5 * size, -0.5 * size);
	  fmoverel (0.5 * size, -0.5 * size);
	  break;
	case M_FANCY_PLUS:	/* ornate plus */
	  fmoverel (-size, 0.0);
	  fcontrel (2 * size, 0.0);
	  fmoverel (-size, -size);
	  fcontrel (0.0, 2 * size);
	  fmoverel (0.5 * size, 0.0);
	  fcontrel (-size, 0.0);
	  fmoverel (-0.5 * size, -0.5 * size);
	  fcontrel (0.0, -size);
	  fmoverel (0.5 * size, -0.5 * size);
	  fcontrel (size, 0.0);
	  fmoverel (0.5 * size, 0.5 * size);
	  fcontrel (0.0, size);
	  fmoverel (-size, 0.0);
	  break;
	case M_FANCY_CROSS:	/* ornate cross */
	  fmoverel (-size, -size);
	  fcontrel (2 * size, 2 * size);
	  fmoverel (0.0, -2 * size);
	  fcontrel (-2 * size, 2 * size);
	  fmoverel (2 * size, -0.5 * size);
	  fcontrel (-0.5 * size, 0.5 * size);
	  fmoverel (-size, 0.0);
	  fcontrel (-0.5 * size, -0.5 * size);
	  fmoverel (0.0, -size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fmoverel (size, 0.0);
	  fcontrel (0.5 * size, 0.5 * size);
	  fmoverel (-size, 0.5 * size);
	  break;
	case M_FANCY_SQUARE:	/* ornate square */
	  fill(0);
	  fboxrel (-0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  fcontrel (0.5 * size, 0.5 * size);
	  fmoverel (-1.5 * size, -1.5 * size);	  
	  fcontrel (-0.5 * size, -0.5 * size);
	  fmoverel (1.5 * size, 0.5 * size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fmoverel (-1.5 * size, 1.5 * size);	  
	  fcontrel (-0.5 * size, 0.5 * size);
	  fmoverel (1.5 * size, -1.5 * size);	  
	  break;
	case M_FANCY_DIAMOND:	/* diamond */
	  fill(0);
	  fmoverel (0.5 * size, 0.0);
	  fcontrel (-0.5 * size, 0.5 * size);
	  fcontrel (-0.5 * size, -0.5 * size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fcontrel (0.5 * size, 0.5 * size);
	  endpoly();
	  fcontrel (0.5 * size, 0.0);
	  fmoverel (-size, 0.5 * size);
	  fcontrel (0.0, 0.5 * size);
	  fmoverel (-0.5 * size, -size);
	  fcontrel (-0.5 * size, 0.0);
	  fmoverel (size, -0.5 * size);
	  fcontrel (0.0, -0.5 * size);
	  fmoverel (0.0, size);
	  break;
	case M_FILLED_FANCY_SQUARE:	/* filled ornate square */
	  fill(1);
	  fboxrel (-0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  fcontrel (0.5 * size, 0.5 * size);
	  fmoverel (-1.5 * size, -1.5 * size);	  
	  fcontrel (-0.5 * size, -0.5 * size);
	  fmoverel (1.5 * size, 0.5 * size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fmoverel (-1.5 * size, 1.5 * size);	  
	  fcontrel (-0.5 * size, 0.5 * size);
	  fmoverel (1.5 * size, -1.5 * size);	  
	  break;
	case M_FILLED_FANCY_DIAMOND: /* filled ornate diamond */
	  fill(1);
	  fmoverel (0.5 * size, 0.0);
	  fcontrel (-0.5 * size, 0.5 * size);
	  fcontrel (-0.5 * size, -0.5 * size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fcontrel (0.5 * size, 0.5 * size);
	  endpoly();
	  fcontrel (0.5 * size, 0.0);
	  fmoverel (-size, 0.5 * size);
	  fcontrel (0.0, 0.5 * size);
	  fmoverel (-0.5 * size, -size);
	  fcontrel (-0.5 * size, 0.0);
	  fmoverel (size, -0.5 * size);
	  fcontrel (0.0, -0.5 * size);
	  fmoverel (0.0, size);
	  break;
	case M_HALF_FILLED_SQUARE:	/* half_filled square */
	  fill(NOMINAL_HALF);
	  fboxrel (-size, -size, size, size);
	  fmoverel (-size, -size);
	  break;
	case M_HALF_FILLED_DIAMOND:	/* half_filled diamond */
	  fill(NOMINAL_HALF);
	  fmoverel (0.0, -size);
	  fcontrel (size, size);
	  fcontrel (-size, size);
	  fcontrel (-size, -size);
	  fcontrel (size, -size);
	  fmoverel (0.0, size);
	  break;
	case M_HALF_FILLED_TRIANGLE: /* half_filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    fill(NOMINAL_HALF);
	    fmoverel (0.0, size);
	    fcontrel (halfwidth, -1.5 * size);
	    fcontrel (-2 * halfwidth, 0.0);
	    fcontrel (halfwidth, 1.5 * size);
	    fmoverel (0.0, -size);
	  }	  
	  break;
	case M_HALF_FILLED_INVERTED_TRIANGLE: /* half_filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    fill(NOMINAL_HALF);
	    fmoverel (0.0, -size);
	    fcontrel (halfwidth, 1.5 * size);
	    fcontrel (-2 * halfwidth, 0.0);
	    fcontrel (halfwidth, -1.5 * size);
	    fmoverel (0.0, size);
	  }	  
	  break;
	case M_HALF_FILLED_CIRCLE:	/* half_filled circle */
	  fill(NOMINAL_HALF);
	  fcirclerel (0.0, 0.0, size);
	  break;
	case M_HALF_FILLED_FANCY_SQUARE:  /* half-filled ornate square */
	  fill(NOMINAL_HALF);
	  fboxrel (-0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  fcontrel (0.5 * size, 0.5 * size);
	  fmoverel (-1.5 * size, -1.5 * size);	  
	  fcontrel (-0.5 * size, -0.5 * size);
	  fmoverel (1.5 * size, 0.5 * size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fmoverel (-1.5 * size, 1.5 * size);	  
	  fcontrel (-0.5 * size, 0.5 * size);
	  fmoverel (1.5 * size, -1.5 * size);	  
	  break;
	case M_HALF_FILLED_FANCY_DIAMOND: /* half-filled ornate diamond */
	  fill(NOMINAL_HALF);
	  fmoverel (0.5 * size, 0.0);
	  fcontrel (-0.5 * size, 0.5 * size);
	  fcontrel (-0.5 * size, -0.5 * size);
	  fcontrel (0.5 * size, -0.5 * size);
	  fcontrel (0.5 * size, 0.5 * size);
	  endpoly();
	  fcontrel (0.5 * size, 0.0);
	  fmoverel (-size, 0.5 * size);
	  fcontrel (0.0, 0.5 * size);
	  fmoverel (-0.5 * size, -size);
	  fcontrel (-0.5 * size, 0.0);
	  fmoverel (size, -0.5 * size);
	  fcontrel (0.0, -0.5 * size);
	  fmoverel (0.0, size);
	  break;
	case M_OCTAGON:		/* octagon */
	  fill(0);
	  fmoverel (-2.0 * size, size);
	  fcontrel (0.0, -2.0 * size);
	  fcontrel (size, -size);
	  fcontrel (2.0 * size, 0.0);
	  fcontrel (size, size);
	  fcontrel (0.0, 2.0 * size);
	  fcontrel (-size, size);
	  fcontrel (-2.0 * size, 0.0);
	  fcontrel (-size, -size);
	  fmoverel (2.0 * size, -size);
	  break;
	case M_FILLED_OCTAGON:	/* filled octagon */
	  fill(1);
	  fmoverel (-2.0 * size, size);
	  fcontrel (0.0, -2.0 * size);
	  fcontrel (size, -size);
	  fcontrel (2.0 * size, 0.0);
	  fcontrel (size, size);
	  fcontrel (0.0, 2.0 * size);
	  fcontrel (-size, size);
	  fcontrel (-2.0 * size, 0.0);
	  fcontrel (-size, -size);
	  fmoverel (2.0 * size, -size);
	  break;
	}

      /* restore the original values of the five relevant drawing attributes,
	 and free storage */
      linemod (old_line_mode);
      capmod (old_cap_mode);
      joinmod (old_join_mode);
      flinewidth (old_line_width);
      fill (old_fill_level);
      
      free (old_line_mode);
      free (old_cap_mode);
      free (old_join_mode);
    }
  
  return 0;
}

int
marker (x, y, type, size)
     int x, y;
     int type;
     int size;
{
  return fmarker ((double)x, (double)y, type, (double)size);
}
