/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path, i.e., a polyline object, may be constructed
   incrementally by repeated invocations of the cont() method.  (See the
   comments in g_cont.c.)  The construction may be terminated, and the
   polyline object finalized, by an explicit invocation of endpath().

   If endpath() is invoked when no polyline is under construction, it has
   no effect.

   In this generic version, we simply deallocate the storage devoted
   to the polyline. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_endpath (void)
#else
_g_endpath ()
#endif
{
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

  /* reset polyline storage buffer */
  if (_plotter->drawstate->PointsInLine > 0)
    {
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->PointsInLine = 0;
    }
  return 0;
}
