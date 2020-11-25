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
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_g_ffontname (R___(Plotter *_plotter) const char *s)
#else
_g_ffontname (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  char *font_name;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontname: invalid operation");
      return -1;
    }

  /* Null pointer resets to default.  (N.B. we don't look at the font_name
     field in _default_drawstate, because it's a dummy.) */
  if ((s == NULL) || (*s == '\0') || !strcmp(s, "(null)"))
    switch (_plotter->default_font_type)
      {
      case F_HERSHEY:
      default:
	s = DEFAULT_HERSHEY_FONT;
	break;
      case F_POSTSCRIPT:
	s = DEFAULT_POSTSCRIPT_FONT;
	break;
      case F_PCL:
	s = DEFAULT_PCL_FONT;
	break;
      case F_STICK:
	s = DEFAULT_STICK_FONT;
	break;
      }

  /* save new font name */
  free ((char *)_plotter->drawstate->font_name);
  font_name = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (font_name, s);
  _plotter->drawstate->font_name = font_name;

  /* retrieve font; compute `true' font size (may differ) */
  _plotter->retrieve_font (S___(_plotter));

  /* return value is size in user units */
  return _plotter->drawstate->true_font_size;
}
