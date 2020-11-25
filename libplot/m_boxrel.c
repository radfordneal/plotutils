/* This file contains the boxrel method, which is a GNU extension to
   libplot.  boxrel() is simply a version of box() that uses relative
   coordinates.  It draws an object: an upright rectangle with specified
   diagonal corners. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_boxrel (int x0, int y0, int x1, int y1)
#else
_m_boxrel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("boxrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d\n", 
		 (int)O_BOXREL, x0, y0, x1, y1);
      else
	{
	  putc ((int)O_BOXREL, _plotter->outstream);
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
_m_fboxrel (double x0, double y0, double x1, double y1)
#else
_m_fboxrel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fboxrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %g %g\n", 
		 (int)O_BOXREL, x0, y0, x1, y1);
      else
	{
	  putc ((int)O_FBOXREL, _plotter->outstream);
	  _emit_float (x0);
	  _emit_float (y0);
	  _emit_float (x1);
	  _emit_float (y1);
	}
    }
  
  return 0;
}
