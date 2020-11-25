/* This file contains the label method, which is a standard part of libplot
   (supplied for backward compatibility).  It draws a label, i.e. a text
   string, at the current location of the graphics device cursor.  It is
   obsoleted by the alabel method, which allows justification. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_label (const char *s)
#else
_m_label (s)
    const char *s;
#endif
{
  char *nl;

  if (!_plotter->open)
    {
      _plotter->error ("label: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      /* null pointer handled specially */
      if (s == NULL)
	s = "(null)";
      
      if ((nl = strchr (s, '\n')))
	*nl = '\0';		/* don't grok multiline arg strings */
      
      fprintf (_plotter->outstream, "%c%s\n", (int)O_LABEL, s);
    }

  return 0;
}
