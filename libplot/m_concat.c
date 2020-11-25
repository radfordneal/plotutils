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
_m_fconcat (R___(Plotter *_plotter) double m0, double m1, double m2, double m3, double m4, double m5)
#else
_m_fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5)
     S___(Plotter *_plotter;) 
     double m0, m1, m2, m3, m4, m5;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fconcat: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_FCONCAT);
  _meta_emit_float (R___(_plotter) m0);
  _meta_emit_float (R___(_plotter) m1);
  _meta_emit_float (R___(_plotter) m2);
  _meta_emit_float (R___(_plotter) m3);
  _meta_emit_float (R___(_plotter) m4);
  _meta_emit_float (R___(_plotter) m5);
  _meta_emit_terminator (S___(_plotter));

  /* invoke generic method to update stored transformation matrix */
  return _g_fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);
}
