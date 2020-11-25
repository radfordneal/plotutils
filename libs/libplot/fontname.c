/* This file contains the fontname routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the name of the font used for
   text subsequent drawn on the graphics device.  The selected font may
   also be changed by calling the fontname routine.

   The fontsize and fontname routines return the fontsize in user units, as
   an aid to vertical positioning by the user.  (The fontsize is normally
   taken to be an acceptable vertical spacing between adjacent lines of
   text.)  A return value of zero means that no information on font size is
   available.

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence on the arguments given to the
   space() routine. */

/* In raw libplot, we return 0, since we have no information about the
   mapping that will ultimately be performed. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* FONTNAME takes a string argument S containing the name of the desired
   current font and sets the current font to that name. */

double
ffontname (s)
     char *s;
{
  char *t, *nl;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fontname() called when graphics device not open\n");
      return -1;
    }

  /* invalid pointer handled specially */
  if (s == NULL)
    s = "(null)";

  /* copy because we may alter the string */
  t = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  if ((nl = strchr (t, '\n')))
    *nl = '\0';			/* don't grok multiline font names */

  fprintf (_outstream, "%c%s\n", 
	   FONTNAME, t);
  free (t);

  return 0.0;
}

int 
fontname (s)
     char *s;
{
  double new_size = ffontname (s);
  
  return IROUND(new_size);
}
