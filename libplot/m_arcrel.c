/* This file contains the arcrel() method, which is a GNU extension to
   libplot.  arcrel() is simply a version of arc() that uses relative
   coordinates.  It draws an object: a counterclockwise arc with specified
   center, beginning, and end. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_arcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_arcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("arcrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d %d %d\n", 
		 ARCREL, xc, yc, x0, y0, x1, y1);
      else
	{
	  putc (ARCREL, _plotter->outstream);
	  _emit_integer (xc);
	  _emit_integer (yc);
	  _emit_integer (x0);
	  _emit_integer (y0);
	  _emit_integer (x1);
	  _emit_integer (y1);
	}
    }

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_farcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_farcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("farcrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream == NULL)
    return 0;

  if (_plotter->portable_output)
    fprintf (_plotter->outstream, "%c %g %g %g %g %g %g\n", 
	     ARCREL, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (FARCREL, _plotter->outstream);
      _emit_float (xc);
      _emit_float (yc);
      _emit_float (x0);
      _emit_float (y0);
      _emit_float (x1);
      _emit_float (y1);
    }

  return 0;
}
