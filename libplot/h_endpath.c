/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated invocation of the cont() routine.  (See the comments in
   g_cont.c.)  The construction may be terminated, and the polyline object
   finalized, by an explict invocation of endpath().

   If endpath() is invoked when no polyline is under construction, it has
   no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_endpath (void)
#else
_h_endpath ()
#endif
{
  IntPoint *xarray;
  Point saved_pos;
  bool closed, use_polygon_buffer;
  int i, polyline_len;

  /* If a circular arc has been stashed rather than drawn, force it to be
     drawn by invoking farc() with the `immediate' flag set.  Note that
     if an arc is stashed, PointsInLine must be zero. */
  if (_plotter->drawstate->arc_stashed) 
    { 
      double axc = _plotter->drawstate->axc;
      double ayc = _plotter->drawstate->ayc;
      double ax0 = _plotter->drawstate->ax0;
      double ay0 = _plotter->drawstate->ay0;
      double ax1 = _plotter->drawstate->ax1;
      double ay1 = _plotter->drawstate->ay1;

      _plotter->drawstate->arc_immediate = true; 
      _plotter->drawstate->arc_polygonal = false; /* advisory only */
      _plotter->farc (axc, ayc, ax0, ay0, ax1, ay1);
      _plotter->drawstate->arc_immediate = false;
      _plotter->drawstate->arc_stashed = false;
    }

  saved_pos = _plotter->drawstate->pos; /* current graphics cursor position */

  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->PointsInLine == 0)	/* nothing to do */
    return 0;
  if (_plotter->drawstate->PointsInLine == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;
      return 0;
    }
  
  if ((_plotter->drawstate->PointsInLine >= 3) /* check for closure */
      && (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1].x == _plotter->drawstate->datapoints[0].x)
      && (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1].y == _plotter->drawstate->datapoints[0].y))
    closed = true;
  else
    closed = false;		/* 2-point ones should be open */
  
  /* Special case: disconnected points, no real polyline.  We switch to a
     temporary datapoints buffer for this.  This is a hack, needed because
     the fcircle() method calls endpath(), which would otherwise mess the
     real databuffer up. */
  if (!_plotter->drawstate->points_are_connected)
    {
      Point *saved_datapoints = _plotter->drawstate->datapoints;
      int saved_PointsInLine = _plotter->drawstate->PointsInLine;
      double radius = 0.5 * _plotter->drawstate->line_width;
      
      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;

      _plotter->savestate();
      _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			   _plotter->drawstate->fgcolor.green, 
			   _plotter->drawstate->fgcolor.blue);
      _plotter->filltype (1);
      _plotter->linewidth (0);

      _plotter->drawstate->points_are_connected = true;
      for (i = 0; i < saved_PointsInLine - (closed ? 1 : 0); i++)
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
  
  /* General case: points are vertices of a polyline, >=2 points in all */
  
  /* convert vertices to integer device coordinates, removing runs */
  xarray = (IntPoint *)_plot_xmalloc (_plotter->drawstate->PointsInLine * sizeof(IntPoint));
  polyline_len = 0;
  for (i = 0; i < _plotter->drawstate->PointsInLine; i++)
    {
      int device_x, device_y;
	  
      device_x = IROUND(XD(_plotter->drawstate->datapoints[i].x, 
			   _plotter->drawstate->datapoints[i].y));
      device_y = IROUND(YD(_plotter->drawstate->datapoints[i].x, 
			   _plotter->drawstate->datapoints[i].y));
	  
      if ((polyline_len == 0) 
	  || (device_x != xarray[polyline_len-1].x) 
	  || (device_y != xarray[polyline_len-1].y))
	{
	  xarray[polyline_len].x = device_x;
	  xarray[polyline_len].y = device_y;
	  polyline_len++;
	}
    }

  /* Check for special subcase: more than one point in the polyline, but
     they were all mapped to a single integer HP-GL pixel.  If so, we draw
     the path as a filled circle, of diameter equal to the line width.
     Provided that the cap mode isn't "butt", that is. */

  if (_plotter->drawstate->cap_type != CAP_BUTT)
    {
      /* if all points mapped to a single pixel... */
      if (_plotter->drawstate->PointsInLine > 1 && polyline_len == 1)
	{
	  Point *saved_datapoints = _plotter->drawstate->datapoints;
	  double radius = 0.5 * _plotter->drawstate->line_width;
	  
	  /* switch to temporary points buffer (same reason as above) */
	  _plotter->drawstate->datapoints = NULL;
	  _plotter->drawstate->datapoints_len = 0;
	  _plotter->drawstate->PointsInLine = 0;

	  _plotter->savestate();
	  _plotter->fillcolor (_plotter->drawstate->fgcolor.red, 
			       _plotter->drawstate->fgcolor.green, 
			       _plotter->drawstate->fgcolor.blue);
	  _plotter->filltype (1);
	  _plotter->linewidth (0);
	  
	  /* draw single filled circle */
	  _plotter->fcircle (saved_datapoints[0].x, saved_datapoints[0].y, 
			     radius);
	  _plotter->restorestate ();

	  /* free temp storage and polyline buffer */
	  free (xarray);
	  free (saved_datapoints);
	  _plotter->drawstate->pos = saved_pos;

	  /* and that's all */
	  return 0;
	}
    }

  /* will draw vectors into polygon buffer if appropriate */
  use_polygon_buffer = (_plotter->hpgl_version == 2
			|| (_plotter->hpgl_version == 1 /* i.e. "1.5" */
			    && (polyline_len > 2
				|| _plotter->drawstate->fill_level)) ? true : false);
  
  /* Sync pen color.  This is needed here only if HPGL_VERSION is 1, but we
     always do it here so that HP-GL/2 output that draws a polyline, if
     sent erroneously to a generic HP-GL device, will yield a polyline in
     the correct color, so long as the color isn't white. */
  _plotter->set_pen_color ();

  /* set_pen_color() sets the advisory bad_pen flag if white pen (pen #0)
     would have been selected, and we can't use pen #0 to draw with.  Such
     a situation isn't fatal if HPGL_VERSION is "1.5" or "2", since we may
     be filling the polyline with a nonwhite color, as well as using a
     white pen to draw it.  But if HPGL_VERSION is "1", we don't fill
     polylines, so we might as well punt right now. */
  if (_plotter->bad_pen && _plotter->hpgl_version == 1)
    {
      /* free integer storage buffer */
      free (xarray);
      
      /* move to end of polyline */
      _plotter->drawstate->pos 
	= _plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1];

      /* reset polyline storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;

      return 0;
    }

  /* move pen to p0, sync attributes, incl. pen width if possible */
  _plotter->drawstate->pos = _plotter->drawstate->datapoints[0];
  _plotter->set_position();
  _plotter->set_attributes();

  if (use_polygon_buffer)
    /* have a polygon buffer, and can use it to fill polyline */
    {
      /* enter polygon mode */
      strcpy (_plotter->page->point, "PM0;");
      _update_buffer (_plotter->page);
    }

  /* ensure that pen is down for drawing */
  if (_plotter->pendown == false)
    {
      strcpy (_plotter->page->point, "PD;");
      _update_buffer (_plotter->page);
      _plotter->pendown = true;
    }
  
  /* draw polyline as a sequence of >=1 pen advances */
  strcpy (_plotter->page->point, "PA");
  _update_buffer (_plotter->page);

  for (i=1; i < polyline_len - 1; i++)
    {
      sprintf (_plotter->page->point, "%d,%d,", xarray[i].x, xarray[i].y);
      _update_buffer (_plotter->page);
    }
  /* final point ends with semicolon */
  sprintf (_plotter->page->point, "%d,%d;", 
	   xarray[polyline_len - 1].x, xarray[polyline_len - 1].y);
  _update_buffer (_plotter->page);
  
  if (use_polygon_buffer)
    /* using polygon mode; will employ polygon buffer to do filling
       (possibly) and edging */
    {
      if (!closed)
	/* polyline is open, so lift pen and exit polygon mode */
	{
	  strcpy (_plotter->page->point, "PU;");
	  _update_buffer (_plotter->page);
	  _plotter->pendown = false;
	  strcpy (_plotter->page->point, "PM2;");
	  _update_buffer (_plotter->page);
	}
      else
	/* polyline is closed, so exit polygon mode and then lift pen */
	{
	  strcpy (_plotter->page->point, "PM2;");
	  _update_buffer (_plotter->page);
	  strcpy (_plotter->page->point, "PU;");
	  _update_buffer (_plotter->page);
	  _plotter->pendown = false;
	}

      if (_plotter->drawstate->fill_level)
	/* ideally, polyline should be filled */
	{
	  /* Sync fill color.  This may set the _plotter->bad_pen flag (if
	     optimal pen is #0 and we're not allowed to use pen #0 to draw
	     with).  So we test _plotter->bad_pen before using the pen to
	     fill with. */
	  _plotter->set_fill_color ();
	  if (_plotter->bad_pen == false)
	    /* fill the polyline */
	    {
	      strcpy (_plotter->page->point, "FP;");
	      _update_buffer (_plotter->page);
	    }
	}

      /* Sync pen color.  This may set the _plotter->bad_pen flag (if
	 optimal pen is #0 and we're not allowed to use pen #0 to draw
	 with).  So we test _plotter->bad_pen before using the pen. */
      _plotter->set_pen_color ();
      if (_plotter->bad_pen == false)
	/* select appropriate pen for edging, and edge the polyline */
	{
	  _plotter->set_pen_color ();
	  strcpy (_plotter->page->point, "EP;");
	  _update_buffer (_plotter->page);
	}
    }
  
  /* We know where the pen now is: if we used a polygon buffer, then
     _plotter->pos is now xarray[0].  If we didn't (as would be the case if
     we're outputting generi HP-GL), then _plotter->pos is now
     xarray[polyline_len - 1].  Unfortunately we can't simply update
     _plotter->pos, because we want the generated HP-GL[/2] code to work
     properly on both HP-GL and HP-GL/2 devices.  So we punt. */
  _plotter->position_is_unknown = true;

  /* free integer storage buffer */
  free (xarray);

  /* move to end of polyline */
  _plotter->drawstate->pos 
    = _plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine - 1];

  /* reset polyline storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->PointsInLine = 0;

  return 0;
}
