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

  if (_plotter->hpgl_version >= 1)
    /* have a polygon buffer, so will use it for filling */
    {
      /* move pen to p0, sync line attributes, incl. pen width */
      _plotter->drawstate->pos = p0;
      _plotter->set_position();
      _plotter->set_attributes();

      /* enter polygon mode */
      strcpy (_plotter->outbuf.current, "PM0;");
      _update_buffer (&_plotter->outbuf);
      /* ensure that pen is down for drawing */
      if (_plotter->pendown == false)
	{
	  strcpy (_plotter->outbuf.current, "PD;");
	  _update_buffer (&_plotter->outbuf);
	  _plotter->pendown = true;
	}
      degrees = 180.0 * angle / M_PI;
      int_degrees = IROUND (degrees);
      if (_plotter->hpgl_version == 1)
	/* use default chord angle */
	{
	  if (degrees == (double)int_degrees)
	    /* draw arc, specify integer sweep angle if possible */
	    sprintf (_plotter->outbuf.current, "AA%d,%d,%d;",
		     IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
		     int_degrees);
	  else
	    sprintf (_plotter->outbuf.current, "AA%d,%d,%.3f;",
		     IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
		     degrees);
	}
      else
	/* HP-GL/2 device, use a reduced chord angle */
	{
	  if (degrees == (double)int_degrees)
	    /* draw arc, specify integer sweep angle if possible */
	    sprintf (_plotter->outbuf.current, "AA%d,%d,%d,2;",
		     IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
		     int_degrees);
	  else
	    sprintf (_plotter->outbuf.current, "AA%d,%d,%.3f,2;",
		     IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
		     degrees);
	}
      _update_buffer (&_plotter->outbuf);
      
      /* lift pen and exit polygon mode */
      strcpy (_plotter->outbuf.current, "PU;");
      _update_buffer (&_plotter->outbuf);
      _plotter->pendown = false;
      strcpy (_plotter->outbuf.current, "PM2;");
      _update_buffer (&_plotter->outbuf);

      /* select appropriate pen and fill the arc */
      if (_plotter->drawstate->fill_level)
	{
	  _plotter->set_fill_color();
	  strcpy (_plotter->outbuf.current, "FP;");
	  _update_buffer (&_plotter->outbuf);
	}

      /* select appropriate pen and edge the arc */
      _plotter->set_pen_color();
      strcpy (_plotter->outbuf.current, "EP;");
      _update_buffer (&_plotter->outbuf);
    }
  else
    /* no polygon buffer */
    {
      /* move pen to p0, sync attributes */
      _plotter->drawstate->pos = p0;
      _plotter->set_position();
      _plotter->set_attributes();

      /* select appropriate pen */
      _plotter->set_pen_color();
      
      /* ensure that pen is down for drawing */
      if (_plotter->pendown == false)
	{
	  strcpy (_plotter->outbuf.current, "PD;");
	  _update_buffer (&_plotter->outbuf);
	  _plotter->pendown = true;
	}
      degrees = 180.0 * angle / M_PI;
      int_degrees = IROUND (degrees);
      /* note: generic HP-GL can only handle integer sweep angles */
      sprintf (_plotter->outbuf.current, "AA%d,%d,%d;",
	       IROUND(XD(pc.x,pc.y)), IROUND(YD(pc.x,pc.y)),
	       int_degrees);
      _update_buffer (&_plotter->outbuf);
    }
  
  /* update our knowledge of pen position */
  if (_plotter->hpgl_version == 0)
    /* due to integer quantization of sweep angle in generic HP-GL, pen may
       be slightly offset afterwards from where it should be */
    {
      double quantized_angle, quantized_angle1;
      double quantized_x1, quantized_y1;
      
      quantized_angle = M_PI * int_degrees / 180.0;
      quantized_angle1 = angle0 + quantized_angle;
      quantized_x1 = pc.x + radius * cos (quantized_angle1);
      quantized_y1 = pc.y + radius * sin (quantized_angle1);      

      _plotter->pos.x = IROUND(XD(quantized_x1,quantized_y1));
      _plotter->pos.y = IROUND(YD(quantized_x1,quantized_y1));
    }
  else
    /* use of polygon buffer implies pen is now at initial point */
    {
      _plotter->pos.x = IROUND(XD(x0,y0));
      _plotter->pos.y = IROUND(YD(x0,y0));
    }
  
  _plotter->drawstate->pos = p1;	/* move to p1 (a libplot convention) */

  return 0;
}
