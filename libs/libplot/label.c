/* This file contains the label routine, which is a standard part of
   libplot.  It draws a label, i.e. a text string, at the current location
   of the graphics device cursor. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
label (s)
    char *s;
{
  char *nl;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: label() called when graphics device not open\n");
      return -1;
    }

  if (s == NULL)		/* avoid core dumps */
    return 0;

  if ((nl = strchr (s, '\n')))
    *nl = '\0';			/* don't grok multiline labels */
  if (*s == '\0')
    return 0;

  fprintf (_outstream, "%c%s\n", LABEL, s);

  return 0;
}
