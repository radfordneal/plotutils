/* This file contains the concat routine, which is a GNU extension to
   libplot.  As in Postscript, it left-multiplies the transformation matrix
   from user coordinates to device coordinates by a specified matrix.  That
   is, it modifies the affine transformation from user coordinates to
   device coordinates, by requiring that the transformation currently in
   effect be be preceded by a specified affine transformation. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fconcat (m0, m1, m2, m3, m4, m5)
     double m0, m1, m2, m3, m4, m5;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fconcat() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g %g %g %g %g\n",
	     FCONCAT, m0, m1, m2, m3, m4, m5);
  else
    {
      putc (FCONCAT, _outstream);
      _putfloat (m0, _outstream);
      _putfloat (m1, _outstream);
      _putfloat (m2, _outstream);
      _putfloat (m3, _outstream);
      _putfloat (m4, _outstream);
      _putfloat (m5, _outstream);
    }  

  return 0;
}
