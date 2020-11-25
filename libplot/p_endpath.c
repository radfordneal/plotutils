/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated invocations of the cont() routine.  (See the comments in
   g_cont.c.)  The construction may be terminated, and the polyline object
   finalized, by an explicit invocation of endpath().

   If endpath() is invoked when no polyline is under construction, it has
   no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* 16-bit brush patterns for idraw (1 = on, 0 = off): */
const long _ps_line_type_bit_vector[] = 
/* solid, dotted, dotdashed, shortdashed, longdashed */
{ 0xffff, 0x8888, 0xc3f0, 0xf0f0, 0xffc0 };

/* corresponding dash arrays for PS (cylically used, on/off/on/off... */
const char *_ps_line_type_setdash[] =
/* these on/off bit patterns are those used by our X11 driver, and also by
   the xterm Tektronix emulator, except that the emulator seems incorrectly
   to have on and off interchanged */
{ "", "1 3", "1 3 4 3", "4 4", "7 4" };

/* PS join styles, indexed by internal join type number (miter/rd./bevel) */
const int _ps_join_style[] =
{ PS_JOIN_MITER, PS_JOIN_ROUND, PS_JOIN_BEVEL };

/* PS cap styles, indexed by internal cap type number (butt/rd./project) */
const int _ps_cap_style[] =
{ PS_CAP_BUTT, PS_CAP_ROUND, PS_CAP_PROJECT };

int
#ifdef _HAVE_PROTOS
_p_endpath (void)
#else
_p_endpath ()
#endif
{
  int i, capstyle, joinstyle, numpoints;
  bool closed, singular_map;
  double linewidth, invnorm = 0.0, granularity = 1.0;

  if (!_plotter->open)
    {
      _plotter->error ("endpath: invalid operation");
      return -1;
    }

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
      Point saved_pos;
      Point *saved_datapoints = _plotter->drawstate->datapoints;
      int saved_PointsInLine = _plotter->drawstate->PointsInLine;
      
      saved_pos = _plotter->drawstate->pos;

      _plotter->drawstate->datapoints = NULL;
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;
      _plotter->drawstate->points_are_connected = true; /* for duration */

      for (i = 0; i < saved_PointsInLine - (closed ? 1 : 0); i++)
	_plotter->fpoint (saved_datapoints[i].x,
			  saved_datapoints[i].y);

      free (saved_datapoints);
      _plotter->drawstate->points_are_connected = false;
      _plotter->drawstate->pos = saved_pos; /* restore graphics cursor */
      return 0;
    }
  
  /* general case: points are vertices of a polyline */

  /* compute reciprocal norm of user->device affine transformation; need
     this because transformation matrices we'll emit must be normalized */
  {
    double det, norm;
    
    det = _plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3]
      - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2];
    norm = sqrt(fabs(det));
    /* granularity = scaleup factor for user coordinates, so that when
       they're emitted as integers, resolution loss won't be excessive.
       CTM entries will be scaled down by this factor. */
    granularity = norm / (PS_MIN_RESOLUTION);
    if (norm != 0.0)
      {
	/* invnorm is `norm' of device->user coordinate transformation */
	invnorm = 1.0 / norm;
	singular_map = false;
      }
    else
      singular_map = true;
  }

  /* includes idraw instruction: start of MLine or Poly */
  if (closed)
    strcpy (_plotter->outbuf.current, "Begin %I Poly\n");
  else
    strcpy (_plotter->outbuf.current, "Begin %I MLine\n");
  _update_buffer (&_plotter->outbuf);

  /* redefine `originalCTM' matrix, which is the CTM applied when the
     polyline is stroked (as opposed to drawn).  We define it to be the
     same as the one in effect when the polyline was drawn. */
  if (singular_map != true)
    {
      int integer_linewidth = _plotter->drawstate->quantized_device_line_width;
      double double_linewidth = _plotter->drawstate->device_line_width;
      double linewidth_adjust;

      /* adjustment needed, due to our specifying line widths as integers */
      if (integer_linewidth != 0)
	linewidth_adjust = double_linewidth / integer_linewidth;
      else
	linewidth_adjust = 1.0;

      sprintf (_plotter->outbuf.current, "[");
      _update_buffer (&_plotter->outbuf);

      for (i = 0; i < 4; i++)
	{
	  sprintf (_plotter->outbuf.current, "%.7g ", 
		   linewidth_adjust * invnorm * _plotter->drawstate->transform.m[i]);
	  _update_buffer (&_plotter->outbuf);
	}
      _update_buffer (&_plotter->outbuf);
      sprintf (_plotter->outbuf.current, 
	       "0 0 ] trueoriginalCTM originalCTM\n concatmatrix pop\n");
      _update_buffer (&_plotter->outbuf);
    }

  /* specify cap style and join style */
  sprintf (_plotter->outbuf.current, "%d setlinecap %d setlinejoin\n",
	   _ps_cap_style[_plotter->drawstate->cap_type], 
	   _ps_join_style[_plotter->drawstate->join_type]);
  _update_buffer (&_plotter->outbuf);

  /* idraw instruction: line type bit vector */
  sprintf (_plotter->outbuf.current, "%%I b %ld\n", 
	   _ps_line_type_bit_vector[_plotter->drawstate->line_type]);
  _update_buffer (&_plotter->outbuf);

  /* PS instruction: SetB (i.e. setbrush), with args
     LineWidth, LeftArrow, RightArrow, DashArray, DashOffset. */
  /* Note LineWidth must be an integer for idraw compatibility. */
  sprintf (_plotter->outbuf.current, "%d 0 0 [ %s ] 0 SetB\n", 
	   _plotter->drawstate->quantized_device_line_width,
	   _ps_line_type_setdash[_plotter->drawstate->line_type]);
  _update_buffer (&_plotter->outbuf);
  
  /* idraw instruction: set foreground color */
  _plotter->set_pen_color();  /* invoked lazily, i.e. when needed */
  sprintf (_plotter->outbuf.current, "%%I cfg %s\n%g %g %g SetCFg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red, 
	   _plotter->drawstate->ps_fgcolor_green, 
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer (&_plotter->outbuf);
  
  /* idraw instruction: set background color */
  _plotter->set_fill_color();  /* invoked lazily, i.e. when needed */
  sprintf (_plotter->outbuf.current, "%%I cbg %s\n%g %g %g SetCBg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_bgcolor],
	   _plotter->drawstate->ps_fillcolor_red, 
	   _plotter->drawstate->ps_fillcolor_green, 
	   _plotter->drawstate->ps_fillcolor_blue);
  _update_buffer (&_plotter->outbuf);
  
  /* includes idraw instruction: set fill pattern */
  if (_plotter->drawstate->fill_level == 0)	/* transparent */
    sprintf (_plotter->outbuf.current, "%%I p\nnone SetP\n");
  else	/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_plotter->outbuf.current, "%%I p\n%f SetP\n", 
	     _idraw_stdshadings[_plotter->drawstate->idraw_shading]);
  _update_buffer (&_plotter->outbuf);
  
  /* includes idraw instruction: transformation matrix (all 6 elements) */
  sprintf (_plotter->outbuf.current, "%%I t\n[");
  _update_buffer (&_plotter->outbuf);
  for (i = 0; i < 6; i++)
    {
      if ((i==0) || (i==1) || (i==2) || (i==3))
	sprintf (_plotter->outbuf.current, "%.7g ", _plotter->drawstate->transform.m[i] / granularity);
      else
	sprintf (_plotter->outbuf.current, "%.7g ", _plotter->drawstate->transform.m[i]);
      _update_buffer (&_plotter->outbuf);
    }
  sprintf (_plotter->outbuf.current, "] concat\n");
  _update_buffer (&_plotter->outbuf);
  
  /* idraw instruction: number of points in line */
  sprintf (_plotter->outbuf.current, "%%I %d\n", 
	   _plotter->drawstate->PointsInLine - (closed ? 1 : 0));
  _update_buffer (&_plotter->outbuf);

  linewidth = _plotter->drawstate->line_width;
  capstyle = _ps_cap_style[_plotter->drawstate->cap_type];
  joinstyle = _ps_join_style[_plotter->drawstate->join_type];
  /* number of points to be output */
  numpoints = _plotter->drawstate->PointsInLine - (closed ? 1 : 0);
  for (i=0; i < numpoints; i++)
    {
      /* output the data point */
      sprintf (_plotter->outbuf.current, "%d %d\n",
	       IROUND(granularity * (_plotter->drawstate->datapoints[i]).x),
	       IROUND(granularity * (_plotter->drawstate->datapoints[i]).y));
      _update_buffer (&_plotter->outbuf);
      
      /* update bounding box */
      if (!closed && ((i == 0) || (i == numpoints - 1)))
	/* an end rather than a join */
	{
	  int otherp;
	  
	  otherp = (i == 0 ? 1 : numpoints - 2);
	  _set_line_end_bbox (_plotter->drawstate->datapoints[i].x,
			      _plotter->drawstate->datapoints[i].y,
			      _plotter->drawstate->datapoints[otherp].x,
			      _plotter->drawstate->datapoints[otherp].y,
			      linewidth, capstyle);
	}
      else
	/* a join rather than an end */
	{
	  int a, b, c;

	  if (closed && i == 0)	/* wrap */
	    {
	      a = numpoints - 1;
	      b = 0;
	      c = 1;
	    }
	  else			/* normal join */
	    {
	      a = i - 1;
	      b = i;
	      c = i + 1;
	    }
	  _set_line_join_bbox(_plotter->drawstate->datapoints[a].x,
			      _plotter->drawstate->datapoints[a].y,
			      _plotter->drawstate->datapoints[b].x,
			      _plotter->drawstate->datapoints[b].y,
			      _plotter->drawstate->datapoints[c].x,
			      _plotter->drawstate->datapoints[c].y,
			      linewidth, joinstyle);
	}
    }
  
  if (closed)
    sprintf (_plotter->outbuf.current, "%d Poly\nEnd\n\n", numpoints);
  else
    sprintf (_plotter->outbuf.current, "%d MLine\nEnd\n\n", numpoints);
  _update_buffer (&_plotter->outbuf);
  
  /* reset polyline storage buffer */
  free (_plotter->drawstate->datapoints);
  _plotter->drawstate->datapoints_len = 0;
  _plotter->drawstate->PointsInLine = 0;

  return 0;
}
