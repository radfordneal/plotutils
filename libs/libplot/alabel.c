/* This file contains the alabel routine, which is a GNU extension to
   libplot.  It draws a label, i.e. a text string, at the current location.
   Horizontal and vertical justification must be specified.

   The alabel routine returns the width of the string in user units.  A
   return value of zero may mean that no information on font size is
   available.

   In raw libplot, we return 0, since we have no information about the
   mapping between user coordinates and device coordinates that will
   ultimately be performed. */

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

double
falabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     char *s;
{
  char x_justify_c, y_justify_c;
  char *t, *nl;
  
  x_justify_c = (char)x_justify;
  y_justify_c = (char)y_justify;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: falabel() called when graphics device not open\n");
      return -1;
    }

  if (s == NULL)		/* avoid core dumps */
    return 0.0;

  /* copy because we may alter the string */
  t = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  if ((nl = strchr (t, '\n')))
    *nl = '\0';			/* don't grok multiline labels */
  if (*t == '\0')
    {
      free (t);
      return 0.0;
    }

  fprintf(_outstream, "%c%c%c%s\n", 
	  ALABEL, x_justify_c, y_justify_c, t);
  free (t);
  
  return 0.0;
}

int
alabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     char *s;
{
  double width = falabel (x_justify, y_justify, s);
  
  return IROUND(width);
}
