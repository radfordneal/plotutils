/* This file contains the move method, which is a standard part of libplot.
   It sets a drawing attribute: the location of the graphics cursor, which
   determines the position of the next object drawn on the graphics
   device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_move (int x, int y)
#else
_m_move (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("move: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d\n", 
		 MOVE, x, y);
      else
	{
	  putc (MOVE, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmove (double x, double y)
#else
_m_fmove (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmove: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n", 
		 MOVE, x, y);
      else
	{
	  putc (FMOVE, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}
    }
  
  return 0;
}
