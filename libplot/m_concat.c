/* This file contains the concat method, which is a GNU extension to
   libplot.  As in Postscript, it left-multiplies the transformation matrix
   from user coordinates to device coordinates by a specified matrix.  That
   is, it modifies the affine transformation from user coordinates to
   device coordinates, by requiring that the transformation currently in
   effect be be preceded by a specified affine transformation. */

#include "sys-defines.h"
#include "plot.h"
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

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g %g %g %g\n",
		 FCONCAT, m0, m1, m2, m3, m4, m5);
      else
	{
	  putc (FCONCAT, _plotter->outstream);
	  _emit_float (m0);
	  _emit_float (m1);
	  _emit_float (m2);
	  _emit_float (m3);
	  _emit_float (m4);
	  _emit_float (m5);
	}  
    }

  /* invoke generic method */
  return _g_fconcat (m0, m1, m2, m3, m4, m5);
}
