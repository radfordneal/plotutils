/* This file contains the alabel method, which is a GNU extension to
   libplot.  It draws a label, i.e. a text string, at the current location.
   Horizontal and vertical justification must be specified. */

#include "sys-defines.h"
#include "plot.h"
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
  char x_justify_c, y_justify_c;
  char *nl;
  
  x_justify_c = (char)x_justify;
  y_justify_c = (char)y_justify;

  if (!_plotter->open)
    {
      _plotter->error ("alabel: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      /* null pointer handled specially */
      if (s == NULL)
	s = "(null)";
      
      if ((nl = strchr (s, '\n')))
	*nl = '\0';		/* don't grok multiline arg strings */
      
      fprintf(_plotter->outstream, "%c%c%c%s\n", 
	      ALABEL, x_justify_c, y_justify_c, s);
    }
  
  return 0;
}
