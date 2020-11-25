/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_point (int x, int y)
#else
_m_point (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("point: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d\n", 
		 POINT, x, y);
      else
	{
	  putc (POINT, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	}
    }
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fpoint (double x, double y)
#else
_m_fpoint (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n", 
		 POINT, x, y);
      else
	{
	  putc (FPOINT, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}
    }
      
  return 0;
}
