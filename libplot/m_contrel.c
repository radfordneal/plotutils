/* This file contains the contrel() method, which is a GNU extension to
   libplot.  contrel() is simply a version of cont() that uses relative
   coordinates.  It continues a line. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_contrel (int x, int y)
#else
_m_contrel (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("contrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d\n", 
		 CONTREL, x, y);
      else
	{
	  putc (CONTREL, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcontrel (double x, double y)
#else
_m_fcontrel (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcontrel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n", 
		 CONTREL, x, y);
      else
	{
	  putc (FCONTREL, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}
    }
  
  return 0;
}
