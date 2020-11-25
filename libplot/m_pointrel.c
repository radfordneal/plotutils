/* This file contains the pointrel method, which is a GNU extension to
   libplot.  pointrel() is simply a version of point() that uses relative
   coordinates.  It plots an object: a point with specified coordinates. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_pointrel (int x, int y)
#else
_m_pointrel (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("pointrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d\n", 
		 (int)O_POINTREL, x, y);
      else
	{
	  putc ((int)O_POINTREL, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	}
    }
      
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fpointrel (double x, double y)
#else
_m_fpointrel (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fpointrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n", 
		 (int)O_POINTREL, x, y);
      else
	{
	  putc ((int)O_FPOINTREL, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}
    }
      
  return 0;
}
