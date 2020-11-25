/* This file contains the internal paint_path() and paint_paths() methods,
   which the public method endpath() is a wrapper around. */

/* This file also contains the internal path_is_flushable() method, which
   is invoked after any path segment is added to the segment list, provided
   (0) the segment list has become greater than or equal to the
   `max_unfilled_path_length' Plotter parameter, (1) the path isn't to be
   filled.  In most Plotters, this operation simply returns true. */

/* This file also contains the internal maybe_prepaint_segments() method.
   It is called immediately after any segment is added to a path.  Some
   Plotters, at least under some circumstances, treat endpath() as a no-op,
   and plot the segments of a path in real time, instead.  They accomplish
   this by overloading this method. */

#include "sys-defines.h"
#include "extern.h"

/* In a generic Plotter, paint_path() does nothing. */

void
#ifdef _HAVE_PROTOS
_g_paint_path (S___(Plotter *_plotter))
#else
_g_paint_path (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

/* In a generic Plotter, path_is_flushable() simply returns true. */

bool
#ifdef _HAVE_PROTOS
_g_path_is_flushable (S___(Plotter *_plotter))
#else
_g_path_is_flushable (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return true;
}

/* In a generic Plotter, maybe_prepaint_segments() does nothing. */

void
#ifdef _HAVE_PROTOS
_g_maybe_prepaint_segments (R___(Plotter *_plotter) int prev_num_segments)
#else
_g_maybe_prepaint_segments (R___(_plotter) prev_num_segments)
     S___(Plotter *_plotter;)
     int prev_num_segments;
#endif
{
  return;
}

/* In a generic Plotter, paint_paths() does nothing but returns `true'. */

bool
#ifdef _HAVE_PROTOS
_g_paint_paths (S___(Plotter *_plotter))
#else
_g_paint_paths (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return true;
}
