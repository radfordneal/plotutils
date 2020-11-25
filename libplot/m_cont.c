/* This file contains the cont() method, which is a standard part of the
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths.  By repeatedly
   invoking cont(), the user may construct a polyline of arbitrary length.
   arc() and ellarc() may also be invoked, to add circular or elliptic arc
   elements to the path.  The path will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          e.g. by invoking move(), linemod(), linewidth(), pencolor(), 
	  fillcolor(), or filltype(), or 
     (3) draws some non-path object, by invoking box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) invokes restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_cont (R___(Plotter *_plotter) int x, int y)
#else
_m_cont (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "cont: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_CONT);
  _meta_emit_integer (R___(_plotter) x);
  _meta_emit_integer (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_fcont (R___(Plotter *_plotter) double x, double y)
#else
_m_fcont (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcont: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_CONT : (int)O_FCONT);
  _meta_emit_float (R___(_plotter) x);
  _meta_emit_float (R___(_plotter) y);
  _meta_emit_terminator (S___(_plotter));
      
  return 0;
}
