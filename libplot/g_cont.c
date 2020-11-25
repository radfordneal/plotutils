/* This file contains the cont() method, which is a standard part of the
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths, i.e. of polylines.  By
   repeatedly invoking cont(), the user may construct a polyline of
   arbitrary length.  The polyline will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by invoking move(), linemod(), linewidth(), color(), fillcolor(),
          or filltype(), or 
     (3) draws some non-polyline object, by invoking arc(), box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) invokes restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* initial size */
#define DATAPOINTS_BUFSIZ MAX_UNFILLED_POLYLINE_LENGTH

int
#ifdef _HAVE_PROTOS
_g_fcont (double x, double y)
#else
_g_fcont (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcont: invalid operation");
      return -1;
    }

  /* If a circular arc has been stashed rather than drawn, force the
     immediate drawing of a polygonal approximation to it, by repeated
     invocation of fcont() */
  if (_plotter->drawstate->arc_stashed) 
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

  /* create or adjust size of polyline buffer, as needed */
  if (_plotter->drawstate->datapoints_len == 0)
    {
      _plotter->drawstate->datapoints = (Point *) 
	_plot_xmalloc (DATAPOINTS_BUFSIZ * sizeof(Point));
      _plotter->drawstate->datapoints_len = DATAPOINTS_BUFSIZ;
    }
  if (_plotter->drawstate->PointsInLine == _plotter->drawstate->datapoints_len)
    {
      _plotter->drawstate->datapoints = (Point *) 
	_plot_xrealloc (_plotter->drawstate->datapoints, 
			2 * _plotter->drawstate->datapoints_len * sizeof(Point));
      _plotter->drawstate->datapoints_len *= 2;
    }
  
  /* analyse the present situation */

  if (_plotter->drawstate->PointsInLine == 0)
    /* no polyline in progress, so begin one (at current position) */
    {
      _plotter->drawstate->datapoints[0].x = (_plotter->drawstate->pos).x;
      _plotter->drawstate->datapoints[0].y = (_plotter->drawstate->pos).y;
      _plotter->drawstate->PointsInLine++;
    }

  /* add new point to internal polyline buffer, so that PointsInLine >=2 */
  _plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine].x = x;
  _plotter->drawstate->datapoints[_plotter->drawstate->PointsInLine].y = y;
  _plotter->drawstate->PointsInLine++;

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  /* now conduct some tests on length; may invoke endpath() method */

  /* If polyline is getting too long (and it doesn't have to be filled),
     flush it to output and begin a new one.  `Too long' is library
     dependent.  The `suppress polyline flushout' flag is set during the
     drawing of polygonal approximations to ellipses (incl. circles),
     elliptic arcs, and circular arcs; see g_fakearc.c.  Also for some
     Plotters, it may be permanently set. */
  if ((_plotter->drawstate->PointsInLine >= _plotter->max_unfilled_polyline_length)
      && !_plotter->drawstate->suppress_polyline_flushout
      && (_plotter->drawstate->fill_level == 0))
    _plotter->endpath();
  
  /* Check whether we're about to violate the hard length limit on all
     polylines.  (Such a limit is imposed is imposed in an XPlotter,
     because of the X protocol restrictions; for other Plotters they length
     limit is typically MAXINT.) */
  if (_plotter->drawstate->PointsInLine >= _plotter->hard_polyline_length_limit)
    {
      _plotter->warning ("breaking an overly long polyline");
      _plotter->endpath();
    }

  return 0;
}
