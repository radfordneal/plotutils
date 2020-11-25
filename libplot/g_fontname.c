/* This file contains the fontname method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the name of the font used for
   text subsequent drawn on the graphics device.  The selected font may
   also be changed by invoking the fontname method.

   The fontname, fontsize, and textangle methods return the fontsize in
   user units, as an aid to vertical positioning by the user.  (The
   fontsize is normally taken to be a minimum vertical spacing between
   adjacent lines of text.)  A return value of zero means that no
   information on font size is available.

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence, e.g., on the arguments given to
   the space() method. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_g_ffontname (const char *s)
#else
_g_ffontname (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("ffontname: invalid operation");
      return -1;
    }

  /* null pointer resets to default */
  if ((s == NULL) || (*s == '\0') || !strcmp(s, "(null)"))
    s = _plotter->default_drawstate->font_name;

  /* save new font name */
  free (_plotter->drawstate->font_name);
  _plotter->drawstate->font_name = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_plotter->drawstate->font_name, s);

  /* retrieve font; compute `true' font size (may differ) */
  _plotter->retrieve_font();

  /* return value is size in user units */
  return _plotter->drawstate->true_font_size;
}
