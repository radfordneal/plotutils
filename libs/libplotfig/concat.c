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
  double m[6];

  m[0] = m0;
  m[1] = m1;
  m[2] = m2;
  m[3] = m3;
  m[4] = m4;
  m[5] = m5;

  _matrix_product (m, _drawstate->transform.m, _drawstate->transform.m);

  /* set fig_quantized_font_size, and (for PS fonts) fig_font_point_size */
  _set_font_sizes();

  return 0;
}
