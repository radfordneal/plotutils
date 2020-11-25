/* This file contains the label method, which is a standard part of libplot
   (supplied for backward compatibility).  It draws a label, i.e. a text
   string, at the current location of the graphics device cursor.  It is
   obsoleted by the alabel method, which allows justification. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_label (const char *s)
#else
_g_label (s)
    const char *s;
#endif
{
  /* label should have baseline passing through current location, and
     should be left-justified */
  return _plotter->alabel ('l', 'x', s);
}
