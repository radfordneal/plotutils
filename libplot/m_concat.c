/* This file contains the fconcat method, which is a GNU extension to
   libplot.  As in Postscript, it left-multiplies the transformation matrix
   from user coordinates to device coordinates by a specified matrix.  That
   is, it modifies the affine transformation from user coordinates to
   device coordinates, by requiring that the transformation currently in
   effect be be preceded by a specified affine transformation. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_fconcat (double m0, double m1, double m2, double m3, double m4, double m5)
#else
_m_fconcat (m0, m1, m2, m3, m4, m5)
     double m0, m1, m2, m3, m4, m5;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fconcat: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_FCONCAT);
  _meta_emit_float (m0);
  _meta_emit_float (m1);
  _meta_emit_float (m2);
  _meta_emit_float (m3);
  _meta_emit_float (m4);
  _meta_emit_float (m5);
  _meta_emit_terminator ();

  /* invoke generic method to update stored transformation matrix */
  return _g_fconcat (m0, m1, m2, m3, m4, m5);
}

/* This function is called in _g_fconcat() to update the device-frame line
   width when fconcat() is invoked, if the Plotter is a MetaPlotter.  It's
   little more than a no-op, since MetaPlotters have no real notion of
   device-frame line width.  See g_concat.c. */
void
#ifdef _HAVE_PROTOS
_m_recompute_device_line_width (void)
#else
_m_recompute_device_line_width ()
#endif
{
  /* use generic method, not MetaPlotter-specific linewidth() method */
  _g_flinewidth (_plotter->drawstate->line_width);
}
