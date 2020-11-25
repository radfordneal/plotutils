/* This file contains the translate, rotate and scale methods, which are
   GNU extensions to libplot.  They affect the affine transformation from
   user coordinates to device coordinates, as in Postscript. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_ftranslate (R___(Plotter *_plotter) double x, double y)
#else
_g_ftranslate (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  double m0, m1, m2, m3, m4, m5;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter)
		       "ftranslate: invalid operation");
      return -1;
    }

  m0 = m3 = 1.0;
  m1 = m2 = 0.0;
  m4 = x;
  m5 = y;
  _plotter->fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_frotate (R___(Plotter *_plotter) double theta)
#else
_g_frotate (R___(_plotter) theta)
     S___(Plotter *_plotter;)
     double theta;
#endif
{
  double m0, m1, m2, m3, m4, m5;
  double radians = M_PI * theta / 180.0;
  
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter)
		       "frotate: invalid operation");
      return -1;
    }

  m0 = m3 = cos (radians);
  m1 = sin (radians);
  m2 = - sin (radians);
  m4 = m5 = 0.0;
  _plotter->fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_fscale (R___(Plotter *_plotter) double x, double y)
#else
_g_fscale (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  double m0, m1, m2, m3, m4, m5;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter)
		       "fscale: invalid operation");
      return -1;
    }

  m0 = x;
  m3 = y;
  m1 = m2 = m4 = m5 = 0.0;
  _plotter->fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);

  return 0;
}
