/* This file contains the translate, rotate and scale routines, which are
   GNU extensions to libplot.  They affect the affine transformation from
   user coordinates to device coordinates, as in Postscript. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
ftranslate (x, y)
     double x,y;
{
  double m0, m1, m2, m3, m4, m5;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ftranslate() called when graphics device not open\n");
      return -1;
    }

  m0 = m3 = 1.0;
  m1 = m2 = 0.0;
  m4 = x;
  m5 = y;
  fconcat (m0, m1, m2, m3, m4, m5);
  
  return 0;
}

int
frotate (theta)
     double theta;
{
  double m0, m1, m2, m3, m4, m5;
  double radians = M_PI * theta / 180.0;
  
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: frotate() called when graphics device not open\n");
      return -1;
    }

  m0 = m3 = cos (radians);
  m1 = sin (radians);
  m2 = - sin (radians);
  m4 = m5 = 0.0;
  fconcat (m0, m1, m2, m3, m4, m5);

  return 0;
}

int
fscale (x, y)
     double x, y;
{
  double m0, m1, m2, m3, m4, m5;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fscale() called when graphics device not open\n");
      return -1;
    }

  m0 = x;
  m3 = y;
  m1 = m2 = m4 = m5 = 0.0;
  fconcat (m0, m1, m2, m3, m4, m5);

  return 0;
}
