/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for AIPlotters.  By construction, for AIPlotters our
   path storage buffer includes a sequence of line segments (no other path
   elements such as arcs). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_a_endpath (void)
#else
_a_endpath ()
#endif
{
  int i, numpoints;
  bool closed;
  double linewidth;

  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path == 0)	/* nothing to do */
    return 0;
  if (_plotter->drawstate->points_in_path == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;
      return 0;
    }
  
  if ((_plotter->drawstate->points_in_path >= 3) /* check for closure */
      && (_plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].x == _plotter->drawstate->datapoints[0].x)
      && (_plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].y == _plotter->drawstate->datapoints[0].y))
    closed = true;
  else
    closed = false;		/* 2-point ones should be open */
  
  /* Special case: disconnected points, no real polyline.  We switch to a
     temporary datapoints buffer for this.  This is a hack, needed because
     the fcircle() method calls endpath(), which would otherwise mess the
     real databuffer up. */

  if (!_plotter->drawstate->points_are_connected)
    {
      Point saved_pos;
      GeneralizedPoint *saved_datapoints = _plotter->drawstate->datapoints;
      double radius = 0.5 * _plotter->drawstate->line_width;
      int saved_points_in_path = _plotter->drawstate->points_in_path;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      _plotter->savestate();
      _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (1);
      _plotter->linewidth (0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_points_in_path - (closed ? 1 : 0); i++)
	/* draw each point as a filled circle, diameter = line width */
	_plotter->fcircle (saved_datapoints[i].x, saved_datapoints[i].y, 
			   radius);
      _plotter->drawstate->points_are_connected = false;

      _plotter->restorestate();
      free (saved_datapoints);
      if (closed)
	_plotter->drawstate->pos = saved_pos; /* restore graphics cursor */
      return 0;
    }
  
  /* general case: points are vertices of a polyline */

  /* set fill color and pen color */
  if (_plotter->drawstate->fill_level > 0)
    /* will be filling the path */
    _plotter->set_fill_color();
  else
    /* won't be filling the path, but set AI's fill color anyway;
       in particular set it to be the same as the pen color (this is a
       convenience for AI users who may wish e.g. to switch from stroking
       to filling) */
    {
      Color old_fillcolor;

      old_fillcolor = _plotter->drawstate->fillcolor;
      _plotter->drawstate->fillcolor = _plotter->drawstate->fgcolor;
      _plotter->set_fill_color();
      _plotter->drawstate->fillcolor = old_fillcolor;
    }
  _plotter->set_pen_color();

  /* update line attributes (cap style, join style, line width),
     if necessary */
  _plotter->set_attributes();

  linewidth = _plotter->drawstate->line_width;
  numpoints = _plotter->drawstate->points_in_path;

  for (i = 0; i < numpoints; i++)
    {
      /* output the data point */
      sprintf (_plotter->page->point, 
	       "%.4f %.4f ", 
	       XD((_plotter->drawstate->datapoints)[i].x,
		  (_plotter->drawstate->datapoints)[i].y),
	       YD((_plotter->drawstate->datapoints)[i].x,
		  (_plotter->drawstate->datapoints)[i].y));
      _update_buffer (_plotter->page);
      if (i == 0)
	/* start of path, move to point */
	sprintf (_plotter->page->point, "m\n");
      else
	/* append line segment to path */
	sprintf (_plotter->page->point, "L\n");
      _update_buffer (_plotter->page);

      /* update bounding box */
      if (!closed && ((i == 0) || (i == numpoints - 1)))
	/* an end rather than a join */
	{
	  int otherp;
	  
	  otherp = (i == 0 ? 1 : numpoints - 2);
	  _set_line_end_bbox (_plotter->page,
			      _plotter->drawstate->datapoints[i].x,
			      _plotter->drawstate->datapoints[i].y,
			      _plotter->drawstate->datapoints[otherp].x,
			      _plotter->drawstate->datapoints[otherp].y,
			      linewidth, _plotter->drawstate->cap_type);
	}
      else
	/* a join rather than an end */
	{
	  int a, b, c;

	  if (closed && i == 0)	/* wrap */
	    {
	      a = numpoints - 2;
	      b = 0;
	      c = 1;
	    }
	  else if (closed && i == numpoints - 1) /* wrap */
	    {
	      a = numpoints - 2;
	      b = numpoints - 1;
	      c = 0;
	    }
	  else			/* normal join */
	    {
	      a = i - 1;
	      b = i;
	      c = i + 1;
	    }
	  _set_line_join_bbox(_plotter->page,
			      _plotter->drawstate->datapoints[a].x,
			      _plotter->drawstate->datapoints[a].y,
			      _plotter->drawstate->datapoints[b].x,
			      _plotter->drawstate->datapoints[b].y,
			      _plotter->drawstate->datapoints[c].x,
			      _plotter->drawstate->datapoints[c].y,
			      linewidth, _plotter->drawstate->join_type);
	}
    }

  /* emit `closepath' if path is closed; stroke and maybe fill */
  if (_plotter->drawstate->fill_level > 0)
    {
      if (closed)
	/* close path, fill and stroke */
	sprintf (_plotter->page->point, "b\n");
      else
	/* fill and stroke */
	sprintf (_plotter->page->point, "B\n");
    }
  else
    {
      if (closed)
	/* close path, stroke */
	sprintf (_plotter->page->point, "s\n");
      else
	/* stroke */
	sprintf (_plotter->page->point, "S\n");
    }
  _update_buffer (_plotter->page);

  /* reset path storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->points_in_path = 0;

  return 0;
}
