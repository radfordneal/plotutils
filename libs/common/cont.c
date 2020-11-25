/* This file contains the cont() routine, which is a standard part of the
   libplot.  It continues a line from the last point plotted to the point
   specified by x and y.  If no point was just plotted, it plots one before
   continuing the line.

   This routine is used in the construction of polyline objects.  By
   repeatedly calling cont(), the user may construct a polyline of
   arbitrary length.  The polyline will terminate when the user either

     (1) makes an explict call to the endpoly() routine, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by calling move(), linemod(), linewidth(), color(), fillcolor(),
          or fill(), or 
     (3) draws some non-polyline object, by calling arc(), box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) calls restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* initial size */
#define DATAPOINTS_BUFSIZ MAX_UNFILLED_POLYLINE_LENGTH

/* Default length an unfilled polyline is allowed to grow to, before it is
   flushed out (we put no [small] maximum on the length of filled
   polylines, since they need to be preserved as discrete objects if the
   filling is to be performed properly).  This may be changed by the user. */
int _libplot_max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;

/* not to be changed by user (though it may be set internally, e.g. for
   libplotX it is computed from the maximum X request size) */
long int _hard_polyline_length_limit = MAXINT;

int
fcont (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcont() called when graphics device not open\n");
      return -1;
    }

  /* create or adjust size of polyline buffer, as needed */

  if (_drawstate->datapoints_len == 0)
    {
      _drawstate->datapoints = (Point *) 
	_plot_xmalloc (DATAPOINTS_BUFSIZ * sizeof(Point));
      _drawstate->datapoints_len = DATAPOINTS_BUFSIZ;
    }
  
  if (_drawstate->PointsInLine == _drawstate->datapoints_len)
    {
      _drawstate->datapoints = (Point *) 
	_plot_xrealloc (_drawstate->datapoints, 
			2 * _drawstate->datapoints_len * sizeof(Point));
      _drawstate->datapoints_len *= 2;
    }
  
  /* analyse the present situation */

  if (_drawstate->PointsInLine == 0)
    /* no polyline in progress, so begin one (at current position) */
    {
      _drawstate->datapoints[0].x = (_drawstate->pos).x;
      _drawstate->datapoints[0].y = (_drawstate->pos).y;
      _drawstate->PointsInLine++;
    }

  /* add new point to internal polyline buffer, so that PointsInLine >=2 */
  _drawstate->datapoints[_drawstate->PointsInLine].x = x;
  _drawstate->datapoints[_drawstate->PointsInLine].y = y;
  _drawstate->PointsInLine++;

  /* update our notion of position */
  (_drawstate->pos).x = x;
  (_drawstate->pos).y = y;

  /* If polyline is getting too long (and it doesn't have to be filled),
     flush it to output and begin a new one.  `Too long' is library
     dependent. */
  if ((_drawstate->PointsInLine >= _libplot_max_unfilled_polyline_length)
      && !_suppress_polyline_flushout
      && (_drawstate->fill_level == 0))
    endpoly();
  
  /* Check whether we're about to violate the hard length limit on all
     polylines.  (Such a limit is imposed in libplotX, for example, because
     of the X protocol restrictions.) */
  if (_drawstate->PointsInLine >= _hard_polyline_length_limit)
    {
      fprintf (stderr, "libplot: warning: breaking an overly long polyline\n");
      endpoly();
    }

  return 0;
}

int
cont (x, y)
     int x, y;
{
  return fcont ((double)x, (double)y);
}
