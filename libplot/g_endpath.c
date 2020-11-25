/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect.

   In this generic version, which is intended for Plotters that draw paths
   in real time but nonetheless store them in a path buffer as they are
   drawn, we simply deallocate the storage devoted to the path. */

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

  /* reset path storage buffer */
  if (_plotter->drawstate->points_in_path > 0)
    {
      free (_plotter->drawstate->datapoints);
      _plotter->drawstate->datapoints_len = 0;
      _plotter->drawstate->points_in_path = 0;
    }
  return 0;
}
