/* This file contains the cont() method, which is a standard part of the
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths.  By repeatedly
   invoking cont(), the user may construct a polyline of arbitrary length.
   arc() and ellarc() may also be invoked, to add circular or elliptic arc
   elements to the path.  The path will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by invoking move(), linemod(), linewidth(), color(), fillcolor(),
          or filltype(), or 
     (3) draws some non-path object, by invoking box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) invokes restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_cont (int x, int y)
#else
_m_cont (x, y)
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("cont: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d\n", 
		 (int)O_CONT, x, y);
      else
	{
	  putc ((int)O_CONT, _plotter->outstream);
	  _emit_integer (x);
	  _emit_integer (y);
	}
    }
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcont (double x, double y)
#else
_m_fcont (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fcont: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n", 
		 (int)O_CONT, x, y);
      else
	{
	  putc ((int)O_FCONT, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}
    }
      
  return 0;
}
