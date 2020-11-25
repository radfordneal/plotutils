/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This version is for HPGL and PCL Plotters.  By construction, for such
   Plotters our path storage buffer may include an arbitrary sequence of
   line and circular arc elements.  (For the latter to be included, the map
   from user to device coordinates must be uniform, so that e.g. the angle
   subtended by the arc will be the same in user and device coordinates.) */

#include "sys-defines.h"
#include "extern.h"

#define DIST(p0,p1) (sqrt( ((p0).x - (p1).x)*((p0).x - (p1).x) \
			  + ((p0).y - (p1).y)*((p0).y - (p1).y)))

typedef struct
{
  int x, y;
  int xc, yc;			/* center for S_ARC, control pt. for S_CUBIC */
  int xd, yd;			/* second control point (S_CUBIC only) */
  double angle;			/* subtended angle (S_ARC only) */
  int type;			/* S_LINE or S_ARC or S_CUBIC */
} GeneralizedIntPoint;

int
#ifdef _HAVE_PROTOS
_h_endpath (void)
#else
_h_endpath ()
#endif
{
  GeneralizedIntPoint *xarray;
  Point saved_pos, p0, pp1, pc;
  bool closed, use_polygon_buffer;
  double degrees;
  double last_x, last_y;
  int i, polyline_len;
  int int_degrees;

  saved_pos = _plotter->drawstate->pos; /* current graphics cursor position */

  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path == 0)	/* nothing to do */
    return 0;
  if (_plotter->drawstate->points_in_path == 1)	/* shouldn't happen */
    {
      /* just reset path storage buffer */
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
      GeneralizedPoint *saved_datapoints = _plotter->drawstate->datapoints;
      int saved_points_in_path = _plotter->drawstate->points_in_path;
      double radius = 0.5 * _plotter->drawstate->line_width;
      
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
  
  /* General case: points are vertices of a polyline, >=2 points in all;
     the elements of the polyline may be circular arcs as well as line
     segments. */
  
  /* convert vertices to integer device coordinates, removing runs */

  /* array for points, with positions expressed in integer device coors */
  xarray = (GeneralizedIntPoint *)_plot_xmalloc (_plotter->drawstate->points_in_path * sizeof(GeneralizedIntPoint));

  /* add first point of path to xarray[] (type field is meaningless) */
  xarray[0].x = IROUND(XD(_plotter->drawstate->datapoints[0].x, 
			  _plotter->drawstate->datapoints[0].y));
  xarray[0].y = IROUND(YD(_plotter->drawstate->datapoints[0].x, 
			  _plotter->drawstate->datapoints[0].y));
  polyline_len = 1;
  /* save user coors of last point added to xarray[] */
  last_x = _plotter->drawstate->datapoints[0].x;
  last_y = _plotter->drawstate->datapoints[0].y;  

  for (i = 1; i < _plotter->drawstate->points_in_path; i++)
    {
      GeneralizedPoint datapoint;
      double xuser, yuser, xdev, ydev;
      int device_x, device_y;
	  
      datapoint = _plotter->drawstate->datapoints[i];
      xuser = datapoint.x;
      yuser = datapoint.y;
      xdev = XD(xuser, yuser);
      ydev = YD(xuser, yuser);
      device_x = IROUND(xdev);
      device_y = IROUND(ydev);

      if (device_x != xarray[polyline_len-1].x
	  || device_y != xarray[polyline_len-1].y)
	/* integer device coor(s) changed, so stash point (incl. type field) */
	{
	  int element_type;
	  int device_xc, device_yc;

	  xarray[polyline_len].x = device_x;
	  xarray[polyline_len].y = device_y;
	  element_type = datapoint.type;
	  xarray[polyline_len].type = element_type;

	  if (element_type == S_ARC)
	    /* an arc element, so compute center, subtended angle too */
	    {
	      device_xc = IROUND(XD(datapoint.xc, datapoint.yc));
	      device_yc = IROUND(YD(datapoint.xc, datapoint.yc));
	      xarray[polyline_len].xc = device_xc;
	      xarray[polyline_len].yc = device_yc;
	      p0.x = last_x; 
	      p0.y = last_y;
	      pp1.x = datapoint.x; 
	      pp1.y = datapoint.y; 
	      pc.x = datapoint.xc; 
	      pc.y = datapoint.yc; 
	      xarray[polyline_len].angle = _angle_of_arc(p0, pp1, pc);
	    }
	  else if (element_type == S_CUBIC)
	    /* a cubic Bezier element, so compute control points too */
	    {
	      xarray[polyline_len].xc 
		= IROUND(XD(datapoint.xc, datapoint.yc));
	      xarray[polyline_len].yc 
		= IROUND(YD(datapoint.xc, datapoint.yc));
	      xarray[polyline_len].xd
		= IROUND(XD(datapoint.xd, datapoint.yd));
	      xarray[polyline_len].yd
		= IROUND(YD(datapoint.xd, datapoint.yd));
	    }

	  /* save user coors of last point added to xarray[] */
	  last_x = datapoint.x;
	  last_y = datapoint.y;  
	  polyline_len++;
	}
    }

  /* Check for special subcase: more than one defining point, but they were
     all mapped to a single integer HP-GL pseudo-pixel.  If so, we draw the
     path as a single dot, unless the cap mode is "butt", in which case we
     don't draw anything. */

  if (_plotter->drawstate->points_in_path > 1 && polyline_len == 1)
    /* all points mapped to a single pixel */
    {
      double xx, yy;
      
      xx = _plotter->drawstate->datapoints[0].x;
      yy = _plotter->drawstate->datapoints[0].y;

      free (xarray);
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      if (_plotter->drawstate->cap_type != CAP_BUTT)
	/* draw single dot */
	_plotter->fpoint (xx, yy);

      return 0;
    }

  /* will draw vectors (or arcs) into polygon buffer if appropriate */
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
      
      /* move to end of polyline (why aren't we there already??) */
      _plotter->drawstate->pos.x
	= _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].x;
      _plotter->drawstate->pos.y
	= _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].y;

      /* reset path storage buffer */
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;

      return 0;
    }

  /* sync attributes, incl. pen width if possible; move pen to p0 */
  _plotter->set_attributes();
  _plotter->drawstate->pos.x = _plotter->drawstate->datapoints[0].x;
  _plotter->drawstate->pos.y = _plotter->drawstate->datapoints[0].y;
  _plotter->set_position();

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
  
  /* loop through points in xarray[], emitting HP-GL instructions */
  i = 1;
  while (i < polyline_len)
    {
      switch (xarray[i].type)
	{
	case S_LINE:
	  /* emit one or more pen advances */
	  strcpy (_plotter->page->point, "PA");
	  _update_buffer (_plotter->page);
	  sprintf (_plotter->page->point, "%d,%d", xarray[i].x, xarray[i].y);
	  _update_buffer (_plotter->page);
	  i++;
	  while (i < polyline_len && xarray[i].type == S_LINE)
	    {
	      sprintf (_plotter->page->point, 
		       ",%d,%d", xarray[i].x, xarray[i].y);
	      _update_buffer (_plotter->page);
	      i++;
	    }
	  sprintf (_plotter->page->point, ";");
	  _update_buffer (_plotter->page);	  
	  break;

	case S_CUBIC:
	  /* emit one or more cubic Bezier segments */
	  strcpy (_plotter->page->point, "BZ");
	  _update_buffer (_plotter->page);
	  sprintf (_plotter->page->point, "%d,%d,%d,%d,%d,%d",
		   xarray[i].xc, xarray[i].yc,
		   xarray[i].xd, xarray[i].yd,
		   xarray[i].x, xarray[i].y);
	  _update_buffer (_plotter->page);
	  i++;
	  while (i < polyline_len && xarray[i].type == S_CUBIC)
	    {
	      sprintf (_plotter->page->point, ",%d,%d,%d,%d,%d,%d",
		       xarray[i].xc, xarray[i].yc,
		       xarray[i].xd, xarray[i].yd,
		       xarray[i].x, xarray[i].y);
	      _update_buffer (_plotter->page);
	      i++;
	    }
	  sprintf (_plotter->page->point, ";");
	  _update_buffer (_plotter->page);	  
	  break;

	case S_ARC:
	  /* emit an arc, using integer sweep angle if possible */
	  degrees = 180.0 * xarray[i].angle / M_PI;
	  int_degrees = IROUND (degrees);
	  if (_plotter->hpgl_version > 0) 
	    /* HPGL_VERSION = 1.5 or 2 */
	    {
	      if (degrees == (double)int_degrees)
		sprintf (_plotter->page->point, "AA%d,%d,%d;",
			 xarray[i].xc, xarray[i].yc,
			 int_degrees);
	      else
		sprintf (_plotter->page->point, "AA%d,%d,%.3f;",
			 xarray[i].xc, xarray[i].yc,
			 degrees);
	    }
	  else
	    /* HPGL_VERSION = 1, i.e. generic HP-GL */
	    /* note: generic HP-GL can only handle integer sweep angles */
	    sprintf (_plotter->page->point, "AA%d,%d,%d;",
		     xarray[i].xc, xarray[i].yc,
		     int_degrees);
	  _update_buffer (_plotter->page);
	  i++;
	  break;

	default:
	  /* print error message, exit */
	  break;
	}
    }

  if (use_polygon_buffer)
    /* using polygon mode; will now employ polygon buffer to do filling
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
	     optimal pen is #0 [white] and we're not allowed to use pen #0
	     to draw with).  So we test _plotter->bad_pen before using the
	     pen to fill with. */
	  _plotter->set_fill_color ();
	  if (_plotter->bad_pen == false)
	    /* fill polyline, specifying nonzero winding rule if necessary */
	    {
	      switch (_plotter->drawstate->fill_rule_type)
		{
		case FILL_ODD_WINDING:
		default:
		  strcpy (_plotter->page->point, "FP;");
		  break;
		case FILL_NONZERO_WINDING:		  
		  if (_plotter->hpgl_version == 2)
		    strcpy (_plotter->page->point, "FP1;");
		  else		/* pre-HP-GL/2 doesn't support nonzero rule */
		    strcpy (_plotter->page->point, "FP;");
		  break;
		}
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
     _plotter->hpgl_pos is now xarray[0].  If we didn't (as would be the
     case if we're outputting generic HP-GL), then _plotter->hpgl_pos is
     now xarray[polyline_len - 1].  Unfortunately we can't simply update
     _plotter->hpgl_pos, because we want the generated HP-GL[/2] code to
     work properly on both HP-GL and HP-GL/2 devices.  So we punt. */
  _plotter->hpgl_position_is_unknown = true;

  /* free integer storage buffer */
  free (xarray);

  /* move to end of polyline */
  _plotter->drawstate->pos.x
    = _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].x;
  _plotter->drawstate->pos.y
    = _plotter->drawstate->datapoints[_plotter->drawstate->points_in_path - 1].y;

  /* reset path storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->points_in_path = 0;

  return 0;
}

double
#ifdef _HAVE_PROTOS
_angle_of_arc(Point p0, Point pp1, Point pc)
#else
_angle_of_arc(p0, pp1, pc)
     Point p0, pp1, pc; 
#endif
{
  Vector v0, v1;
  double cross, angle, angle0;

  /* vectors from pc to p0, and pc to pp1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = pp1.x - pc.x;
  v1.y = pp1.y - pc.y;

  /* relative polar angle of p0 */
  angle0 = _xatan2 (v0.y, v0.x);

  /* cross product, zero means points are collinear */
  cross = v0.x * v1.y - v1.x * v0.y;

  if (cross == 0.0)
    /* by libplot convention, sweep angle should be M_PI not -(M_PI), in
       the collinear case */
    angle = M_PI;
  else
    /* compute angle in range -(M_PI)..M_PI */
    {
      double angle1;

      angle1 = _xatan2 (v1.y, v1.x);
      angle = angle1 - angle0;
      if (angle > M_PI)
	angle -= (2.0 * M_PI);
      else if (angle < -(M_PI))
	angle += (2.0 * M_PI);
    }
  
  /* if user coors -> device coors includes a reflection, flip sign */
  if (!_plotter->drawstate->transform.nonreflection)
    angle = -angle;

  return angle;
}
