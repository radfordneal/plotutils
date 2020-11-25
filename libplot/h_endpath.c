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
  int i;
  bool closed;
  Point saved_pos;

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
     (in this library) point() calls endpath(), which would mess the real
     databuffer up, if we didn't do something like this. */
  if (!_plotter->drawstate->points_are_connected)
    {
      Point *saved_datapoints = _plotter->drawstate->datapoints;
      int saved_PointsInLine = _plotter->drawstate->PointsInLine;
      
      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;
      _plotter->drawstate->points_are_connected = true; /* for duration */

      for (i = 0; i < saved_PointsInLine - (closed ? 1 : 0); i++)
	_plotter->fpoint (saved_datapoints[i].x,
			  saved_datapoints[i].y);

      free (saved_datapoints);
      _plotter->drawstate->points_are_connected = false;
      _plotter->drawstate->pos = saved_pos; /* restore graphics cursor position */
      return 0;
    }
  
  /* general case: points are vertices of a polyline, >=2 points in all */

  /* move pen to first point */
  _plotter->drawstate->pos = _plotter->drawstate->datapoints[0];

  /* sync pen position and line attributes, incl. pen width */
  _plotter->set_position();
  _plotter->set_attributes();

  if (_plotter->hpgl_version >= 1)
    /* have a polygon buffer, and will use it for filling polygon */
    {
      /* enter polygon mode */
      strcpy (_plotter->outbuf.current, "PM0;");
      _update_buffer (&_plotter->outbuf);
    }
  else
    /* no polygon buffer, so will draw directly: select appropriate pen */
    _plotter->set_pen_color ();
    
  /* ensure that pen is down for drawing */
  if (_plotter->pendown == false)
    {
      strcpy (_plotter->outbuf.current, "PD;");
      _update_buffer (&_plotter->outbuf);
      _plotter->pendown = true;
    }
  
  /* draw polyline as a sequence of >=1 pen advances */
  strcpy (_plotter->outbuf.current, "PA");
  _update_buffer (&_plotter->outbuf);

  for (i=1; i < _plotter->drawstate->PointsInLine - 1; i++)
    {
      sprintf (_plotter->outbuf.current, "%d,%d,",
	       IROUND(XD((_plotter->drawstate->datapoints[i]).x,
			 (_plotter->drawstate->datapoints[i]).y)),
	       IROUND(YD((_plotter->drawstate->datapoints[i]).x,
			 (_plotter->drawstate->datapoints[i]).y)));
      _update_buffer (&_plotter->outbuf);
    }
  sprintf (_plotter->outbuf.current, "%d,%d;",
	   IROUND(XD((_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).x,
		     (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).y)),
	   IROUND(YD((_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).x,
		     (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).y)));
  _update_buffer (&_plotter->outbuf);
  
  if (_plotter->hpgl_version >= 1)
    /* have a polygon buffer, and are using it to fill the polygon */
    {
      if (!closed)
	/* polyline is open, so lift pen and exit polygon mode */
	{
	  strcpy (_plotter->outbuf.current, "PU;");
	  _update_buffer (&_plotter->outbuf);
	  _plotter->pendown = false;
	  strcpy (_plotter->outbuf.current, "PM2;");
	  _update_buffer (&_plotter->outbuf);
	}
      else
	/* polyline is closed, so exit polygon mode and then lift pen */
	{
	  strcpy (_plotter->outbuf.current, "PM2;");
	  _update_buffer (&_plotter->outbuf);
	  strcpy (_plotter->outbuf.current, "PU;");
	  _update_buffer (&_plotter->outbuf);
	  _plotter->pendown = false;
	}

      if (_plotter->drawstate->fill_level)
	/* polyline must be filled */
	{
	  /* select appropriate pen and fill the polyline */
	  _plotter->set_fill_color ();
	  strcpy (_plotter->outbuf.current, "FP;");
	  _update_buffer (&_plotter->outbuf);
	}

      /* select appropriate pen and edge the polyline, too */
      _plotter->set_pen_color ();
      strcpy (_plotter->outbuf.current, "EP;");
      _update_buffer (&_plotter->outbuf);
    }

  /* now update our knowledge of pen position */

  if (_plotter->hpgl_version >= 1)
    /* used the polygon buffer, so pen is now at 1st point */
    {
      _plotter->pos.x = IROUND(XD((_plotter->drawstate->datapoints[0]).x,
				   (_plotter->drawstate->datapoints[0]).y));
      _plotter->pos.y = IROUND(YD((_plotter->drawstate->datapoints[0]).x,
				   (_plotter->drawstate->datapoints[0]).y));
    }
  else
    /* didn't use polygon buffer, so pen is now at last point */
    {
      _plotter->pos.x = IROUND(XD((_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).x,
				   (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).y));
      _plotter->pos.y = IROUND(YD((_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).x,
				   (_plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine-1]).y));
    }
      
  /* reset polyline storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->PointsInLine = 0;

  _plotter->drawstate->pos = saved_pos; /* restore graphics cursor position */

  return 0;
}
