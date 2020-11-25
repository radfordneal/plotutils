/* This file contains the cont() routine, which is a standard part of
   libplot.  It continues a line from the last point plotted to the point
   specified by x and y.  If no point was just plotted, it plots one before
   continuing the line.

   This routine is used in the construction of polyline objects.  By
   repeatedly calling cont(), the user may construct a polyline of
   arbitrary length.  The polyline will terminate when the user either

     (1) makes an explict call to the endpoly() routine, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by calling move(), linemod(), linewidth(), color(), fillcolor(),
          or fill(), or 
     (3) draws some non-polyline object, by calling arc(), box(), 
           circle(), point(), label(), or alabel(), or 
     (4) calls restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
cont (x, y)
     int x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: cont() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d %d\n", 
	     CONT, x, y);
  else
    {
      putc (CONT, _outstream);
      _putshort (x, _outstream);
      _putshort (y, _outstream);
    }
  
  return 0;
}

int
fcont (x, y)
     double x, y;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcont() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g %g\n", 
	     FCONT, x, y);
  else
    {
      putc (FCONT, _outstream);
      _putfloat (x, _outstream);
      _putfloat (y, _outstream);
    }
  
  return 0;
}
