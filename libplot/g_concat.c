/* This file contains the concat method, which is a GNU extension to
   libplot.  As in Postscript, it left-multiplies the transformation matrix
   from user coordinates to device coordinates by a specified matrix.  That
   is, it modifies the affine transformation from user coordinates to
   device coordinates, by requiring that the transformation currently in
   effect be be preceded by a specified affine transformation. */

/* Invoking concat causes the device-frame line width and the device-frame
   size of the current font to be recomputed. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_fconcat (double m0, double m1, double m2, double m3, double m4, double m5)
#else
_g_fconcat (m0, m1, m2, m3, m4, m5)
     double m0, m1, m2, m3, m4, m5;
#endif
{
  double m[6];
  double det;

  if (!_plotter->open)
    {
      _plotter->error ("fconcat: invalid operation");
      return -1;
    }

  m[0] = m0;
  m[1] = m1;
  m[2] = m2;
  m[3] = m3;
  m[4] = m4;
  m[5] = m5;

  _matrix_product (m, _plotter->drawstate->transform.m, _plotter->drawstate->transform.m);

  /* following code as in space.c */

  /* does map preserve axis directions? */
  _plotter->drawstate->transform.axes_preserved = 
    (_plotter->drawstate->transform.m[1] == 0.0 
     && _plotter->drawstate->transform.m[2] == 0.0) ? true : false;

#define FUZZ 0.0000001		/* potential roundoff error */
#define IS_ZERO(arg) (IS_ZERO1(arg) && IS_ZERO2(arg))
#define IS_ZERO1(arg) (fabs(arg) < FUZZ * DMAX(_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[0], _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[1]))
#define IS_ZERO2(arg) (fabs(arg) < FUZZ * DMAX(_plotter->drawstate->transform.m[2] * _plotter->drawstate->transform.m[2], _plotter->drawstate->transform.m[3] * _plotter->drawstate->transform.m[3]))
  /* if row vectors are of equal length and orthogonal... */
  if (IS_ZERO(_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[0]
	      + _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[1]
	      - _plotter->drawstate->transform.m[2] * _plotter->drawstate->transform.m[2]
	      - _plotter->drawstate->transform.m[3] * _plotter->drawstate->transform.m[3])
      &&
      IS_ZERO(_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[2] + 
	      _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[3]))
    _plotter->drawstate->transform.uniform = true; /* map's scaling is uniform */
  else
    _plotter->drawstate->transform.uniform = false; /* map's scaling not uniform */

  /* determine whether map involves a reflection, by computing determinant */
  det = (_plotter->drawstate->transform.m[0] *
	 _plotter->drawstate->transform.m[3]
	 - (_plotter->drawstate->transform.m[1] *
	    _plotter->drawstate->transform.m[2]));
  _plotter->drawstate->transform.nonreflection 
    = ((_plotter->flipped_y ? -1 : 1) * det >= 0) ? true : false;

  /* This is a bit of a botch.  We recompute device-frame line width, which
     incidentally calls `endpath'.  But we don't do so if the Plotter on
     which this method is invoked is a MetaPlotter, since doing so would
     cause a bogus LINEWIDTH op code to be emitted to the metafile, and a
     MetaPlotter has no notion of device-frame line width anyway.  */

  if (_plotter->type != PL_META)
    _plotter->flinewidth (_plotter->drawstate->line_width);

  /* recompute font size in device coordinates */
  _plotter->retrieve_font();

  return 0;
}
