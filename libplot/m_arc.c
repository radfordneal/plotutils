/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_arc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
_m_arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("arc: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d %d %d\n", 
		 ARC, xc, yc, x0, y0, x1, y1);
      else
	{
	  putc (ARC, _plotter->outstream);
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
_m_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
_m_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("farc: invalid operation");
      return -1;
    }

  if (_plotter->outstream == NULL)
    return 0;

  if (_plotter->portable_output)
    fprintf (_plotter->outstream, "%c %g %g %g %g %g %g\n", 
	     ARC, xc, yc, x0, y0, x1, y1);
  else
    {
      putc (FARC, _plotter->outstream);
      _emit_float (xc);
      _emit_float (yc);
      _emit_float (x0);
      _emit_float (y0);
      _emit_float (x1);
      _emit_float (y1);
    }
  
  return 0;
}
