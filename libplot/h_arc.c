/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

#define VLENGTH(v) sqrt( (v).x * (v).x + (v).y * (v).y )

int
#ifdef _HAVE_PROTOS
_h_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_h_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  Point p0, p1, pc;
  Vector v0, v1;
  double radius, cross, angle, degrees, angle0;
  int int_degrees;

  if (!_plotter->open)
    {
      _plotter->error ("farc: invalid operation");
      return -1;
    }

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x1,y1).  Only the endpoints will appear on the
     display.  If linemode is "disconnected", no arc-stashing (see below)
     ever occurs. */
  if (!_plotter->drawstate->points_are_connected)
    return _plotter->fline (x0, y0, x1, y1);

  /* Non-immediate case: an arc was previously stashed rather than drawn,
     so first flush it out by drawing a polygonal approximation to it.  We
     do this by setting the `immediate' flag and making a recursive call. */
  if (_plotter->drawstate->arc_stashed
      && !_plotter->drawstate->arc_immediate)
    {
      double axc = _plotter->drawstate->axc;
      double ayc = _plotter->drawstate->ayc;
      double ax0 = _plotter->drawstate->ax0;
      double ay0 = _plotter->drawstate->ay0;
      double ax1 = _plotter->drawstate->ax1;
      double ay1 = _plotter->drawstate->ay1;

      _plotter->drawstate->arc_immediate = true;
      _plotter->drawstate->arc_polygonal = true;
      _plotter->farc (axc, ayc, ax0, ay0, ax1, ay1);
      _plotter->drawstate->arc_immediate = false;
      _plotter->drawstate->arc_stashed = false;
    }

  /* Non-immediate case: if new arc not contiguous, move to its starting
     point (thereby finalizing the path under construction, if any, since
     the move() method invokes the endpath() method). */
  if ((!_plotter->drawstate->arc_immediate)
      && (x0 != _plotter->drawstate->pos.x 
	  || y0 != _plotter->drawstate->pos.y))
    _plotter->fmove (x0, y0);
  
  if (!_plotter->drawstate->arc_immediate
      && _plotter->drawstate->PointsInLine == 0)
    /* Non-immediate case, with no polyline under construction:
       don't actually draw the arc, just stash it for later drawing */
    {
      _plotter->drawstate->axc = xc;
      _plotter->drawstate->ayc = yc;      
      _plotter->drawstate->ax0 = x0;
      _plotter->drawstate->ay0 = y0;      
      _plotter->drawstate->ax1 = x1;
      _plotter->drawstate->ay1 = y1;      
      _plotter->drawstate->arc_stashed = true;
      return 0;
    }

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  /* Immediate case, or non-immediate case with a polyline under
     construction.  We actually draw the arc.  In the immediate case we
     check the advisory `arc_polygonal' flag to determine whether to draw a
     polygonal approximation.  But we only check it if the map from the
     user frame to the device frame is isotropic. */

  /* if affine map from user frame to device frame is anisotropic, draw a
     polygonal approximation by invoking fcont() repeatedly */
  if (!_plotter->drawstate->transform.uniform
      || (_plotter->drawstate->arc_immediate
	  && _plotter->drawstate->arc_polygonal)
      || _plotter->drawstate->PointsInLine > 0)
    {
      _plotter->drawstate->arc_stashed = false;	/* avoid infinite recursion */
      _draw_circular_arc (p0, p1, pc);
      return 0;
    }

  /* otherwise use HP-GL's native arc-drawing facility, as follows */

  if (x0 == x1 && y0 == y1)
    /* zero-length arc */
    {
      _plotter->fmove (x0, y0);
      _plotter->fcont (x0, y0);
      _plotter->endpath (); /* flush out zero-length arc */

      _plotter->drawstate->pos = p1; /* move to p1 (a libplot convention) */

      return 0;
    }

  /* genuine arc, adjust pc as needed */
  pc = _truecenter (p0, p1, pc);
  
  radius = DIST(pc, p0);	/* radius is distance to p0 or p1 */

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

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

  /* Sync pen color.  This is needed here only if HPGL_VERSION is 1, but we
     always do it here so that HP-GL/2 output that draws an arc, if sent
     erroneously to a generic HP-GL device, will yield an arc in the
     correct color, so long as the color isn't white. */
  _plotter->set_pen_color ();

  /* set_pen_color() sets the advisory bad_pen flag if white pen (pen #0)
     would have been selected, and we can't use pen #0 to draw with.  Such
     a situation isn't fatal if HPGL_VERSION is "1.5" or "2", since we may
     be filling the arc with a nonwhite color, as well as using a white pen
     to draw it.  But if HPGL_VERSION is "1", we don't fill arcs, so we
     might as well punt right now. */
  if (_plotter->bad_pen && _plotter->hpgl_version == 1)
    {
      _plotter->drawstate->pos = p1; /* move to p1 (a libplot convention) */
      return 0;
    }

  /* move pen to p0, sync attributes, incl. pen width if possible */
  _plotter->drawstate->pos = p0;
  _plotter->set_position();
  _plotter->set_attributes();

  if (_plotter->hpgl_version >= 1)
    /* have a polygon buffer, and will use it */
    {
      /* enter polygon mode */
      strcpy (_plotter->page->point, "PM0;");
      _update_buffer (_plotter->page);
      /* ensure that pen is down for drawing */
      if (_plotter->pendown == false)
	{
	  strcpy (_plotter->page->point, "PD;");
	  _update_buffer (_plotter->page);
	  _plotter->pendown = true;
	}

      /* draw arc, specify integer sweep angle if possible */
      degrees = 180.0 * angle / M_PI;
      int_degrees = IROUND (degrees);
      if (degrees == (double)int_degrees)
	sprintf (_plotter->page->point, "AA%d,%d,%d;",
		 IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
		 int_degrees);
      else
	sprintf (_plotter->page->point, "AA%d,%d,%.3f;",
		 IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
		 degrees);
      _update_buffer (_plotter->page);
      
      /* lift pen and exit polygon mode */
      strcpy (_plotter->page->point, "PU;");
      _update_buffer (_plotter->page);
      _plotter->pendown = false;
      strcpy (_plotter->page->point, "PM2;");
      _update_buffer (_plotter->page);

      if (_plotter->drawstate->fill_level)
	/* ideally, arc should be filled */
	{
	  /* Sync fill color.  This may set the _plotter->bad_pen flag (if
	     optimal pen is #0 and we're not allowed to use pen #0 to draw
	     with).  So we test _plotter->bad_pen before using the pen to
	     fill with. */
	  _plotter->set_fill_color ();
	  if (_plotter->bad_pen == false)
	    /* fill the arc */
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
	/* select appropriate pen for edging, and edge the arc */
	{
	  _plotter->set_pen_color ();
	  strcpy (_plotter->page->point, "EP;");
	  _update_buffer (_plotter->page);
	}
    }
  else
    /* generic HP-GL, don't have a polygon buffer, won't do filling */
    {
      /* ensure that pen is down for drawing */
      if (_plotter->pendown == false)
	{
	  strcpy (_plotter->page->point, "PD;");
	  _update_buffer (_plotter->page);
	  _plotter->pendown = true;
	}
      degrees = 180.0 * angle / M_PI;
      int_degrees = IROUND (degrees);
      /* note: generic HP-GL can only handle integer sweep angles */
      sprintf (_plotter->page->point, "AA%d,%d,%d;",
	       IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
	       int_degrees);
      _update_buffer (_plotter->page);
    }
  
  /* We know where the pen now is.  If hpgl_version>=1 (i.e. HP7550A or
     HP-GL/2), we used a polygon buffer, so _plotter->pos is now
     (IROUND(XD(x0,y0)), IROUND(YD(x0,y0))).  In generic HP-GL, pen would
     be at the endpoint (IROUND(XD(x1,y1)), IROUND(YD(x1,y1))) except it
     may be slightly offset due to quantization of sweep angle.
     Unfortunately we can't simply update _plotter->pos, because we want
     the generated HP-GL[/2] code to work properly on both HP-GL and
     HP-GL/2 devices.  So we punt. */
  _plotter->position_is_unknown = true;

  _plotter->drawstate->pos = p1;	/* move to p1 (a libplot convention) */

  return 0;
}
