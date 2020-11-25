/* This file contains the label method, which is a standard part of libplot
   (supplied for backward compatibility).  It draws a label, i.e. a text
   string, at the current location of the graphics device cursor.  It is
   obsoleted by the alabel method, which allows justification. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_label (const char *s)
#else
_m_label (s)
    const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("label: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_LABEL);
  _meta_emit_string (s);

  return 0;
}
