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

  /* following code as in space.c */

  /* does map preserve axis directions? */
  _drawstate->transform.axes_preserved = 
    (_drawstate->transform.m[1] == 0.0 
     && _drawstate->transform.m[2] == 0.0);

#define FUZZ 0.0000001		/* potential roundoff error */
#define IS_ZERO(arg) (IS_ZERO1(arg) && IS_ZERO2(arg))
#define IS_ZERO1(arg) (fabs(arg) < FUZZ * DMAX(_drawstate->transform.m[0] * _drawstate->transform.m[0], _drawstate->transform.m[1] * _drawstate->transform.m[1]))
#define IS_ZERO2(arg) (fabs(arg) < FUZZ * DMAX(_drawstate->transform.m[2] * _drawstate->transform.m[2], _drawstate->transform.m[3] * _drawstate->transform.m[3]))
  /* if row vectors are of equal length and orthogonal... */
  if (IS_ZERO(_drawstate->transform.m[0] * _drawstate->transform.m[0]
	      + _drawstate->transform.m[1] * _drawstate->transform.m[1]
	      - _drawstate->transform.m[2] * _drawstate->transform.m[2]
	      - _drawstate->transform.m[3] * _drawstate->transform.m[3])
      &&
      IS_ZERO(_drawstate->transform.m[0] * _drawstate->transform.m[2] + 
	      _drawstate->transform.m[1] * _drawstate->transform.m[3]))
    _drawstate->transform.uniform = TRUE; /* map's scaling is uniform */
  else
    _drawstate->transform.uniform = FALSE; /* map's scaling not uniform */

  /* if current font is an X font, retrieve; also update fields in the
     _drawstate structure */
  _retrieve_font();

  _handle_x_events();

  return 0;
}
