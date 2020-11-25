/* This file contains the markerrel method, which is a GNU extension to
   libplot.  markerrel() is simply a version of marker() that uses relative
   coordinates.  It plots an object: a plotting symbol, or marker, of
   specified type and size. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_markerrel (int x, int y, int type, int size)
#else
_m_markerrel (x, y, type, size)
     int x, y;
     int type;
     int size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("markerrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d\n",
		 (int)O_MARKERREL, x, y, type, size);
      else
	{
	  putc ((int)O_MARKERREL, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	  _emit_integer (type);
	  _emit_integer (size);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fmarkerrel (double x, double y, int type, double size)
#else
_m_fmarkerrel (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmarkerrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %d %g\n",
		 (int)O_MARKERREL, x, y, type, size);
      else
	{
	  putc ((int)O_FMARKERREL, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	  _emit_integer (type);
	  _emit_float (size);
	}
    }
  
  return 0;
}
