/* This file contains the endpoly() routine, which is a GNU extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated calls to the cont() routine.  (See the comments in cont.c.)
   The construction may be terminated, and the polyline object finalized,
   by an explict call to endpoly().

   If endpoly() is called when no polyline is under construction, it
   has no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* whether or not to suppress the flushing out of unfilled polylines when
   they've reached a length MAX_UNFILLED_POLYLINE_LENGTH */

Boolean _suppress_polyline_flushout = FALSE;

int
endpoly ()
{
  int i;
  Boolean closed;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: endpoly() called when graphics device not open\n");
      return -1;
    }

  if (_drawstate->PointsInLine == 0)	/* nothing to do */
    return 0;
  if (_drawstate->PointsInLine == 1)	/* shouldn't happen */
    {
      /* just reset polyline storage buffer */
      free (_drawstate->datapoints);
      _drawstate->datapoints_len = 0;
      _drawstate->PointsInLine = 0;
      return 0;
    }
  
  if ((_drawstate->PointsInLine >= 3) /* check for closure */
      && (_drawstate->datapoints[_drawstate->PointsInLine - 1].x == _drawstate->datapoints[0].x)
      && (_drawstate->datapoints[_drawstate->PointsInLine - 1].y == _drawstate->datapoints[0].y))
    closed = TRUE;
  else
    closed = FALSE;		/* 2-point ones should be open */
  
  /* Special case: disconnected points, no real polyline.  We switch to a
     temporary datapoints buffer for this.  This is a hack, needed because
     (in this library) point() calls endpoly(), which would mess the real
     databuffer up, if we didn't do something like this. */
  if (!_drawstate->points_are_connected)
    {
      Point *saved_datapoints = _drawstate->datapoints;
      int saved_PointsInLine = _drawstate->PointsInLine;
      
      _drawstate->datapoints = NULL;
      _drawstate->datapoints_len = 0;
      _drawstate->PointsInLine = 0;
      _drawstate->points_are_connected = TRUE; /* for duration */

      for (i = 0; i < saved_PointsInLine - (closed ? 1 : 0); i++)
	fpoint (saved_datapoints[i].x,
		saved_datapoints[i].y);

      free (saved_datapoints);
      _drawstate->points_are_connected = FALSE;
      return 0;
    }
  
  /* general case: points are vertices of a polyline */

  /* includes idraw instruction: start of MLine or Poly */
  if (closed)
    strcpy (_outbuf.current, "Begin %I Poly\n");
  else
    strcpy (_outbuf.current, "Begin %I MLine\n");
  _update_buffer (&_outbuf);

  /* idraw instruction: line type bit vector */
  sprintf (_outbuf.current, "%%I b %ld\n", 
	   _ps_line_type_bit_vector[_drawstate->ps_line_type]);
  _update_buffer (&_outbuf);

  /* PS instruction: SetB (i.e. setbrush), with args
     Width, LeftArrow, RightArrow, DashArray, DashOffset */
  sprintf (_outbuf.current, "%d 0 0 [ %s ] 0 SetB\n", 
	   _device_line_width(),
	   _ps_line_type_setdash[_drawstate->ps_line_type]);
  _update_buffer (&_outbuf);
  
  /* idraw instruction: set foreground color */
  _evaluate_ps_fgcolor();  /* evaluated lazily, i.e. when needed */
  sprintf (_outbuf.current, "%%I cfg %s\n%f %f %f SetCFg\n",
	   _idraw_stdcolornames[_drawstate->idraw_fgcolor],
	   _drawstate->ps_fgcolor_red, 
	   _drawstate->ps_fgcolor_green, 
	   _drawstate->ps_fgcolor_blue);
  _update_buffer (&_outbuf);
  
  /* idraw instruction: set background color */
  _evaluate_ps_fillcolor();  /* evaluated lazily, i.e. when needed */
  sprintf (_outbuf.current, "%%I cbg %s\n%f %f %f SetCBg\n",
	   _idraw_stdcolornames[_drawstate->idraw_bgcolor],
	   _drawstate->ps_fillcolor_red, 
	   _drawstate->ps_fillcolor_green, 
	   _drawstate->ps_fillcolor_blue);
  _update_buffer (&_outbuf);
  
  /* includes idraw instruction: set fill pattern */
  if (_drawstate->fill_level == 0)	/* transparent */
    sprintf (_outbuf.current, "%%I p\nnone SetP\n");
  else	/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_outbuf.current, "%%I p\n%f SetP\n", 
	     _idraw_stdshadings[_drawstate->idraw_shading]);
  _update_buffer (&_outbuf);
  
  /* includes idraw instruction: transformation matrix (all 6 elements) */
  sprintf (_outbuf.current, "%%I t\n[");
  _update_buffer (&_outbuf);
  for (i = 0; i < 6; i++)
    {
      if ((i==0) || (i==1) || (i==2) || (i==3))
	sprintf (_outbuf.current, "%f ", _drawstate->transform.m[i] / GRANULARITY);
      else
	sprintf (_outbuf.current, "%f ", _drawstate->transform.m[i]);
      _update_buffer (&_outbuf);
    }
  sprintf (_outbuf.current, "] concat\n");
  _update_buffer (&_outbuf);
  
  /* idraw instruction: number of points in line */
  sprintf (_outbuf.current, "%%I %d\n", 
	   _drawstate->PointsInLine - (closed ? 1 : 0));
  _update_buffer (&_outbuf);

  for (i=0; i<_drawstate->PointsInLine - (closed ? 1 : 0); i++)
    {
      sprintf (_outbuf.current, "%d %d\n",
	       IROUND(GRANULARITY * (_drawstate->datapoints[i]).x),
	       IROUND(GRANULARITY * (_drawstate->datapoints[i]).y));
      _update_buffer (&_outbuf);
      
      _set_range (XD((_drawstate->datapoints[i]).x, (_drawstate->datapoints[i]).y),
		  YD((_drawstate->datapoints[i]).x, (_drawstate->datapoints[i]).y));
    }
  if (closed)
    sprintf (_outbuf.current, "%d Poly\nEnd\n\n", _drawstate->PointsInLine - 1);
  else
    sprintf (_outbuf.current, "%d MLine\nEnd\n\n", _drawstate->PointsInLine);
  _update_buffer (&_outbuf);
  
  /* reset polyline storage buffer */
  free (_drawstate->datapoints);
  _drawstate->datapoints_len = 0;
  _drawstate->PointsInLine = 0;

  return 0;
}
