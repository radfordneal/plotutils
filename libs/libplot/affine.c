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
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ftranslate() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n",
	     FTRANSLATE, x, y);
  else
    {
      putc (FTRANSLATE, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }  

  return 0;
}

int
frotate (theta)
     double theta;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: frotate() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g\n",
	     FROTATE, theta);
  else
    {
      putc (FROTATE, _outstream);
      _putfloat (theta, _outstream);
    }  

  return 0;
}

int
fscale (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fscale() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n",
	     FSCALE, x, y);
  else
    {
      putc (FSCALE, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }  

  return 0;
}
