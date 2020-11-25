/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a plotting symbol, or marker, of specified
   type and size, at a specified location.  This symbol can be one of a
   list of standard symbols, or a single printable ASCII character in the
   current font and fontsize. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_marker (int x, int y, int type, int size)
#else
_m_marker (x, y, type, size)
     int x, y;
     int type;
     int size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("marker: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d %d\n",
		 MARKER, x, y, type, size);
      else
	{
	  putc (MARKER, _plotter->outstream);
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
_m_fmarker (double x, double y, int type, double size)
#else
_m_fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fmarker: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g %d %g\n",
		 MARKER, x, y, type, size);
      else
	{
	  putc (FMARKER, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	  _emit_integer (type);
	  _emit_float (size);
	}
    }
  
  return 0;
}
