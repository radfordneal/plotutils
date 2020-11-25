/* This file contains the endpoly() routine, which is a extension to
   libplot.  A polyline object may be constructed incrementally, by
   repeated calls to the cont() routine.  (See the comments in cont.c.)
   The construction may be terminated, and the polyline object finalized,
   by an explict call to endpoly().

   If endpoly() is called when no polyline is under construction, it
   has no effect. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

/* whether or not to suppress the flushing out of unfilled polylines when
   they've reached a length MAX_UNFILLED_POLYLINE_LENGTH */

Boolean _suppress_polyline_flushout = FALSE;

int
endpoly ()
{
  int i;
  Boolean closed, polyline_subtype;
  char *format;
    
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
     point() in libplotfig calls endpoly(), which would otherwise mess the
     real databuffer up. */
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

  if (closed)
    {
      polyline_subtype = P_CLOSED;
      format = "#POLYLINE [CLOSED]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d";
    }
  else
    {
      polyline_subtype = P_OPEN;
      format = "#POLYLINE [OPEN]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d";
    }

  /* evaluate fig colors lazily, i.e. only when needed */
  _evaluate_fig_fgcolor();
  _evaluate_fig_fillcolor();
  
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_drawstate->fig_last_priority > POLYLINE_PRIORITY)
    if (_drawstate->fig_drawing_depth > 0)
      (_drawstate->fig_drawing_depth)--;
  _drawstate->fig_last_priority = POLYLINE_PRIORITY;

  sprintf(_outbuf.current,
	  format,
	  2,			/* polyline object */
	  polyline_subtype,	/* polyline subtype */
	  _drawstate->fig_line_style, /* style */
	  _device_line_width(), /* thickness, in Fig display units */
	  _drawstate->fig_fgcolor,	/* pen color */
	  _drawstate->fig_fillcolor, /* fill color */
	  _drawstate->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _drawstate->fig_fill_level, /* area fill */
	  _drawstate->fig_dash_length, /* style val, in Fig display units
					   (float) */
	  _drawstate->fig_join_style, /* join style */
	  _drawstate->fig_cap_style, /* cap style */
	  0,			/* radius (of arc boxes, ignored here) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  _drawstate->PointsInLine	/* number of points in line */
	  );
  _update_buffer (&_outbuf);

  for (i=0; i<_drawstate->PointsInLine; i++)
    {

      if ((i%5) == 0)
	sprintf (_outbuf.current, "\n\t"); /* make human-readable */
      else
	sprintf (_outbuf.current, " ");
      _update_buffer (&_outbuf);

      sprintf (_outbuf.current, 
	       "%d %d", 
	       IROUND(XD((_drawstate->datapoints)[i].x,
			 (_drawstate->datapoints)[i].y)),
	       IROUND(YD((_drawstate->datapoints)[i].x,
			 (_drawstate->datapoints)[i].y)));
      _update_buffer (&_outbuf);
    }
  sprintf (_outbuf.current, "\n");
  _update_buffer (&_outbuf);

  /* reset polyline storage buffer */
  free (_drawstate->datapoints);
  _drawstate->datapoints_len = 0;
  _drawstate->PointsInLine = 0;

  return 0;
}
