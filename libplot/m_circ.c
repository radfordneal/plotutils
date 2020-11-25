/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_circle (int x, int y, int r)
#else
_m_circle (x, y, r)
     int x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("circle: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d\n", 
		 (int)O_CIRCLE, x, y, r);
      else
	{
	  putc ((int)O_CIRCLE, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	  _emit_integer (r);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcircle (double x, double y, double r)
#else
_m_fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcircle: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g\n", 
		 (int)O_CIRCLE, x, y, r);
      else
	{
	  putc ((int)O_FCIRCLE, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	  _emit_float (r);
	}
    }
  
  return 0;
}
