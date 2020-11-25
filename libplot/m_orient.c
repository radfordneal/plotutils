/* This file contains the orientation method, which is a GNU extension to
   libplot.  It sets a drawing attribute: whether or not closed paths of
   the three built-in types (rectangles, circles, ellipses) should be drawn
   counterclockwise or clockwise.  The former is the default. 

   NOT YET IMPLEMENTED. */

#include "sys-defines.h"
#include "extern.h" 

int
#ifdef _HAVE_PROTOS
_m_orientation (R___(Plotter *_plotter) int direction)
#else
_m_orientation (R___(_plotter) direction)
     S___(Plotter *_plotter;) 
     int direction;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "orientation: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ORIENTATION);
  _meta_emit_integer (R___(_plotter) direction);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_orientation (R___(_plotter) direction);
}
