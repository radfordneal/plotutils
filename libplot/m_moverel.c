/* This file contains the moverel() method, which is a GNU extension to
   libplot.  moverel() is simply a version of move() that uses relative
   coordinates.  It sets a drawing attribute: the location of the graphics
   cursor, which determines the position of the next object drawn on the
   graphics device. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_moverel (int x, int y)
#else
_m_moverel (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("moverel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d\n", 
		 MOVEREL, x, y);
      else
	{
	  putc (MOVEREL, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmoverel (double x, double y)
#else
_m_fmoverel (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmoverel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n", 
		 MOVEREL, x, y);
      else
	{
	  putc (FMOVEREL, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}
    }
  
  return 0;
}
