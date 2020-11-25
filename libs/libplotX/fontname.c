/* This file contains the fontname routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the name of the font used for
   text subsequent drawn on the graphics device.  The selected font may
   also be changed by calling the fontname routine.

   The fontname, fontsize, and textangle routines return the fontsize in
   user units, as an aid to vertical positioning by the user.  (The
   fontsize is normally taken to be an minimum vertical spacing between
   adjacent lines of text.)  A return value of zero means that no
   information on font size is available.

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence on the arguments given to the
   space() routine. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
ffontname (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ffontname() called when graphics device not open\n");
      return -1;
    }

  /* invalid pointer resets to default */
  if ((s == NULL) || (*s == '\0') || !strcmp(s, "(null)"))
      s = _default_drawstate.font_name;

  if (strcasecmp (_drawstate->font_name, s) != 0) /* font change */
    {
      /* save new font name */
      free (_drawstate->font_name);
      _drawstate->font_name = (char *)_plot_xmalloc (strlen (s) + 1);
      strcpy (_drawstate->font_name, s);
      /* if an X font, retrieve; update fields in the _drawstate structure */
      _retrieve_font();
    }
  
  _handle_x_events();

  /* return value is size in user units */
  return _drawstate->x_font_size;
}
