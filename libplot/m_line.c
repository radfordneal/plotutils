/* This file contains the line method, which is a standard part of libplot.
   It draws an object: a line segment extending from the point x0,y0 to the
   point x1,y1.  By repeatedly invoking cont(), the user may extend this
   line object to a polyline object. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_line (int x0, int y0, int x1, int y1)
#else
_m_line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("line: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf(_plotter->outstream, "%c %d %d %d %d\n", 
		(int)O_LINE, x0, y0, x1, y1);
      else
	{
	  putc ((int)O_LINE, _plotter->outstream);
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
_m_fline (double x0, double y0, double x1, double y1)
#else
_m_fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fline: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf(_plotter->outstream, "%c %g %g %g %g\n", 
		(int)O_LINE, x0, y0, x1, y1);
      else
	{
	  putc ((int)O_FLINE, _plotter->outstream);
	  _emit_float (x0);
	  _emit_float (y0);
	  _emit_float (x1);
	  _emit_float (y1);
	}
    }
  
  return 0;
}
