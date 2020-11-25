/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for AIPlotters.  By construction, for AIPlotters our
   path storage buffer may include only line segments, elliptic arc
   segments, and cubic Bezier segments. */

/* We replace each quarter-ellipse in the path by an approximating cubic
   Bezier.  The approximation is the standard one.  E.g., a quarter circle
   extending from (1,0) to (0,1), with center (0,0), would be approximated
   by a cubic Bezier with control points (1,KAPPA) and (KAPPA,1).  Here
   KAPPA = (4/3)[sqrt(2)-1] = 0.552284749825, approximately.  The cubic
   Bezier will touch the quarter-circle along the line x=y.

   For a quarter-circle, the maximum relative error in r as a function of
   theta is about 2.7e-4.  The error in r has the same sign, for all theta. */

/* [Louis Vosloo <support@yandy.com> points out that the value
   0.55228... for some purposes is sub-optimal.  By dropping the
   requirement that the quarter-circle and the Bezier touch each other
   along the symmetry line x=y, one can decrease the maximum relative
   error.  He says 0.5541... is the best possible choice.] */

/* According to Berthold K. P. Horn <bkph@ai.mit.edu>, the general formula
   for KAPPA, for a radius-1 circular arc (not necessary a quarter-circle)
   to be approximated by a cubic Bezier, is

   KAPPA = (4/3)sqrt[(1-cos H)/(1+cos H)] 
   	 = (4/3)[1-cos H]/[sin H] = (4/3)[sin H]/[1+cosH]

   where H is half the angle subtended by the arc.  H=45 degrees for
   a quarter circle. */

#include "sys-defines.h"
#include "extern.h"

#define KAPPA 0.552284749825

/* Maximum value for squared sine of angle between two segments, if their
   juncture is to be a `smooth point' rather than a corner point.  (In
   Illustrator, smooth points have only one direction handle; not two.) */
#define MAX_SQUARED_SINE (1e-6)

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
  GeneralizedPoint oldpoint, newpoint;

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
  
  /* general case: successive points are endpoints of contiguous line
     and/or cubic Bezier segments */

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

  /* loop over (generalized) points in path, skipping the first (which is
     just a `moveto' repositioning directive) */
  oldpoint = _plotter->drawstate->datapoints[0];
  for (i = 1; i < numpoints; i++)
    {
      newpoint = _plotter->drawstate->datapoints[i];

      switch (newpoint.type)
	{
	case S_LINE:
	default:
	  break;

	case S_ELLARC:
	  /* replace path segments (i.e. `generalized points') that are
	     quarter-ellipses by cubic Beziers, as above */
	  {
	    double xcenter, ycenter, xstart, ystart, xend, yend;
	    double ux, uy, vx, vy;
	    
	    xstart = oldpoint.x;
	    ystart = oldpoint.y;
	    xcenter = newpoint.xc;
	    ycenter = newpoint.yc;
	    xend = newpoint.x;
	    yend = newpoint.y;
	    
	    /* vectors to starting, ending points */
	    ux = xstart - xcenter;
	    uy = ystart - ycenter;	  
	    vx = xend - xcenter;
	    vy = yend - ycenter;
	    
	    /* replace by cubic Bezier, with computed control points */
	    newpoint.type = S_CUBIC;
	    newpoint.xc = xstart + KAPPA * vx;
	    newpoint.yc = ystart + KAPPA * vy;
	    newpoint.xd = xend + KAPPA * ux;
	    newpoint.yd = yend + KAPPA * uy;
	    _plotter->drawstate->datapoints[i] = newpoint;
	  }
	  break;

	case S_ARC:
	  /* replace path segments (i.e. `generalized points') that are
	     circular arcs by cubic Beziers, as above */
	  {
	    int angledir;
	    double xcenter, ycenter, xstart, ystart, xend, yend;
	    double ux, uy, vx, vy;
	    double anglerange, halfangle, sinhalf, coshalf, kappa;

	    xstart = oldpoint.x;
	    ystart = oldpoint.y;
	    xcenter = newpoint.xc;
	    ycenter = newpoint.yc;
	    xend = newpoint.x;
	    yend = newpoint.y;

	    /* vectors to starting, ending points */
	    ux = xstart - xcenter;
	    uy = ystart - ycenter;	  
	    vx = xend - xcenter;
	    vy = yend - ycenter;
	  
	    /* discard degenerate cases (shouldn't really happen) */
	    if ((ux == 0.0 && uy == 0.0) || (vx == 0.0 && vy == 0.0))
	      anglerange = 0.0;
	    else
	      {
		double oldangle, newangle;

		oldangle = _xatan2 (uy, ux);
		newangle = _xatan2 (vy, vx);
		anglerange = newangle - oldangle;
		if (anglerange > M_PI)
		  anglerange -= (2 * M_PI);
		if (anglerange <= -(M_PI))
		  anglerange += (2 * M_PI);
	      }
	    angledir = (anglerange < 0.0 ? -1 : 1); /* -1 means clockwise */

	    halfangle = 0.5 * FABS(anglerange);
	    sinhalf = sin (halfangle);
	    coshalf = cos (halfangle);
	    if (FABS(sinhalf) < 0.5)
	      kappa = (4.0/3.0) * sinhalf / (1.0 + coshalf);
	    else
	      kappa = (4.0/3.0) * (1.0 - coshalf) / sinhalf;

	    /* replace by cubic Bezier, with computed control points */
	    newpoint.type = S_CUBIC;
	    newpoint.xc = xstart - kappa * angledir * uy;
	    newpoint.yc = ystart + kappa * angledir * ux;
	    newpoint.xd = xend + kappa * angledir * vy;
	    newpoint.yd = yend - kappa * angledir * vx;
	    _plotter->drawstate->datapoints[i] = newpoint;
	  }
	  break;
	}
	  
      oldpoint = newpoint;
    }
      
  /* loop over (generalized) points in path */
  for (i = 0; i < numpoints; i++)
    {
      bool smooth_join_point;	/* if a path join point, a smooth one? */

      /* update bounding box to take into account the segment's terminal
	 point (which is either a path join point or a path end point) */

      if (!closed && (i == 0 || i == numpoints - 1))
	/* for the path, an end rather than a join */
	{
	  double xcurrent, ycurrent, xother, yother;
	  
	  smooth_join_point = false;

	  /* compute path end point, and a nearby point, the vector to
	     which will determine the shape of the path end */
	  xcurrent = _plotter->drawstate->datapoints[i].x;
	  ycurrent = _plotter->drawstate->datapoints[i].y;	  

	  if (i == 0)		/* i = 0, initial end point */
	    {
	      if (_plotter->drawstate->datapoints[i+1].type == S_CUBIC)
		{
		  xother = _plotter->drawstate->datapoints[i+1].xc;
		  yother = _plotter->drawstate->datapoints[i+1].yc;
		}
	      else /* line segment */
		{
		  xother = _plotter->drawstate->datapoints[i+1].x;
		  yother = _plotter->drawstate->datapoints[i+1].y;
		}
	    }
	  else			/* i = numpoints - 1, final end point */
	    {
	      if (_plotter->drawstate->datapoints[i].type == S_CUBIC)
		{
		  xother = _plotter->drawstate->datapoints[i].xd;
		  yother = _plotter->drawstate->datapoints[i].yd;
		}
	      else /* line segment */
		{
		  xother = _plotter->drawstate->datapoints[i-1].x;
		  yother = _plotter->drawstate->datapoints[i-1].y;
		}
	    }
	  /* take path end into account: update bounding box */
	  _set_line_end_bbox (_plotter->page,
			      xcurrent, ycurrent, xother, yother,
			      linewidth, _plotter->drawstate->cap_type);
	}
      else
	/* for the path, a join rather than an end */
	{
	  int a, b, c;
	  double xcurrent, ycurrent, xleft, yleft, xright, yright;

	  if (closed && (i == 0 || i == numpoints - 1)) /* wrap */
	    {
	      a = numpoints - 2;
	      b = numpoints - 1;
	      c = 1;
	    }
	  else			/* normal join */
	    {
	      a = i - 1;
	      b = i;
	      c = i + 1;
	    }
	  
	  xcurrent = _plotter->drawstate->datapoints[b].x;
	  ycurrent = _plotter->drawstate->datapoints[b].y;

	  /* compute points to left and right, vectors to which will
	     determine the shape of the path join */
	  switch (_plotter->drawstate->datapoints[b].type)
	    {
	    case S_LINE:
	    default:
	      xleft = _plotter->drawstate->datapoints[a].x;
	      yleft = _plotter->drawstate->datapoints[a].y;
	      break;
	    case S_CUBIC:
	      xleft = _plotter->drawstate->datapoints[b].xd;
	      yleft = _plotter->drawstate->datapoints[b].yd;
	      break;
	    }
	  switch (_plotter->drawstate->datapoints[c].type)
	    {
	    case S_LINE:
	    default:
	      xright = _plotter->drawstate->datapoints[c].x;
	      yright = _plotter->drawstate->datapoints[c].y;
	      break;
	    case S_CUBIC:
	      xright = _plotter->drawstate->datapoints[c].xc;
	      yright = _plotter->drawstate->datapoints[c].yc;
	      break;
	    }
	  
	  /* take path join into account: update bounding box */
	  _set_line_join_bbox(_plotter->page,
			      xleft, yleft, xcurrent, ycurrent, xright, yright,
			      linewidth, 
			      _plotter->drawstate->join_type,
			      _plotter->drawstate->miter_limit);

	  /* is join smooth? */
	  {
	    double ux, uy, vx, vy, cross, dot, uselfdot, vselfdot;
	    
	    ux = xleft - xcurrent;
	    uy = yleft - ycurrent;
	    vx = xright - xcurrent;
	    vy = yright - ycurrent;

	    cross = ux * vy - uy * vx;
	    dot = ux * vx + uy * vy;
	    uselfdot = ux * ux + uy * uy;
	    vselfdot = vx * vx + vy * vy;
	    
	    if (cross * cross < MAX_SQUARED_SINE * uselfdot * vselfdot 
		&& dot < 0.0)
	      smooth_join_point = true;
	    else
	      smooth_join_point = false;
	  }
	}

      /* output to Illustrator the points that define this segment */

      if (i != 0 && (_plotter->drawstate->datapoints)[i].type == S_CUBIC)
	/* cubic Bezier segment, so output control points */
	{
	  sprintf (_plotter->page->point, 
		   "%.4f %.4f %.4f %.4f ", 
		   XD(_plotter->drawstate->datapoints[i].xc,
		      _plotter->drawstate->datapoints[i].yc),
		   YD(_plotter->drawstate->datapoints[i].xc,
		      _plotter->drawstate->datapoints[i].yc),
		   XD(_plotter->drawstate->datapoints[i].xd,
		      _plotter->drawstate->datapoints[i].yd),
		   YD(_plotter->drawstate->datapoints[i].xd,
		      _plotter->drawstate->datapoints[i].yd));
	  _update_buffer (_plotter->page);
	  /* update bounding box due to extremal x/y values in device frame */
	  _set_bezier3_bbox (_plotter->page, 
			     _plotter->drawstate->datapoints[i-1].x,
			     _plotter->drawstate->datapoints[i-1].y,
			     _plotter->drawstate->datapoints[i].xc,
			     _plotter->drawstate->datapoints[i].yc,
			     _plotter->drawstate->datapoints[i].xd,
			     _plotter->drawstate->datapoints[i].yd,
			     _plotter->drawstate->datapoints[i].x,
			     _plotter->drawstate->datapoints[i].y);
	}
      
      /* output terminal point of segment */
      sprintf (_plotter->page->point, 
	       "%.4f %.4f ", 
	       XD(_plotter->drawstate->datapoints[i].x,
		  _plotter->drawstate->datapoints[i].y),
	       YD(_plotter->drawstate->datapoints[i].x,
		  _plotter->drawstate->datapoints[i].y));
      _update_buffer (_plotter->page);
      
      /* tell Illustrator what sort of path segment this is */
      if (i == 0)
	/* start of path, so just move to point */
	sprintf (_plotter->page->point, "m\n");
      else
	/* append line segment or Bezier segment to path */
	switch (_plotter->drawstate->datapoints[i].type)
	  {
	  case S_LINE:
	  default:
	    sprintf (_plotter->page->point, 
		     smooth_join_point ? "l\n" : "L\n");
	    break;
	  case S_CUBIC:
	    sprintf (_plotter->page->point, 
		     smooth_join_point ? "c\n" : "C\n");
	    break;	    
	  }
      _update_buffer (_plotter->page);

    } /* end of loop over (generalized) path points */


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
