/* This file contains the label method, which is a standard part of libplot
   (supplied for backward compatibility).  It draws a label, i.e. a text
   string, at the current location of the graphics device cursor.  It is
   obsoleted by the alabel method, which allows justification. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_label (R___(Plotter *_plotter) const char *s)
#else
_m_label (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  char *t;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "label: invalid operation");
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
  
  _meta_emit_byte (R___(_plotter) (int)O_LABEL);
  _meta_emit_string (R___(_plotter) t);

  free (t);
  
  return 0;
}
