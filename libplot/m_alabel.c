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
_m_alabel (R___(Plotter *_plotter) int x_justify, int y_justify, const char *s)
#else
_m_alabel (R___(_plotter) x_justify, y_justify, s)
     S___(Plotter *_plotter;) 
     int x_justify, y_justify;
     const char *s;
#endif
{
  char *t;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "alabel: invalid operation");
      return -1;
    }

  /* copy because we may alter the string */
  t = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  /* allow only character set in ISO encoding */
  {
    bool was_clean;
    
    was_clean = _clean_iso_string ((unsigned char *)t);
    if (!was_clean)
      _plotter->warning (R___(_plotter) 
			 "ignoring control character (e.g. CR or LF) in label");
  }
  
  _meta_emit_byte (R___(_plotter) (int)O_ALABEL);
  _meta_emit_byte (R___(_plotter) x_justify);
  _meta_emit_byte (R___(_plotter) y_justify);
  _meta_emit_string (R___(_plotter) t);
  
  free (t);

  return 0;
}
