/* This file contains the alabel method, which is a GNU extension to
   libplot.  It draws a label, i.e. a text string, at the current location.
   Horizontal and vertical justification must be specified. */

#include "sys-defines.h"
#include "extern.h"

/* ALABEL takes three arguments X_JUSTIFY, Y_JUSTIFY, and S, and places the
   label S according to the x and y axis adjustments specified in X_JUSTIFY
   and Y_JUSTIFY.  X_JUSTIFY is equal to 'l', 'c', or 'r', signifying
   left-justified, centered, or right-justified, relative to the current
   position.  Y_JUSTIFY is equal to 'b', 'x', 'c', or 't', signifying that
   the bottom, baseline, center, or top of the label should pass through
   the current position. */

int
#ifdef _HAVE_PROTOS
_m_alabel (int x_justify, int y_justify, const char *s)
#else
_m_alabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("alabel: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_ALABEL);
  _meta_emit_byte (x_justify);
  _meta_emit_byte (y_justify);
  _meta_emit_string (s);
  
  return 0;
}
