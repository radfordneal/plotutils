/* This file contains the linemod method, which is a standard part of
   libplot.  It sets a drawing attribute: the line style used in subsequent
   drawing operations.

   This version searches for the specified line style in a table of known
   line styles (see g_dash2.c). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_linemod (const char *s)
#else
_g_linemod (s)
     const char *s;
#endif
{
  bool matched = false;
  int i;

  if (!_plotter->open)
    {
      _plotter->error ("linemod: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.line_mode;

  free (_plotter->drawstate->line_mode);
  _plotter->drawstate->line_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_plotter->drawstate->line_mode, s);

  if (strcmp( s, "disconnected") == 0)
     /* we'll implement disconnected lines by drawing a filled circle at
	each path join point */
    {
      _plotter->drawstate->line_type = L_SOLID;
      _plotter->drawstate->points_are_connected = false;
      matched = true;
    }
  
  else	/* search table of libplot's builtin line styles */
    for (i = 0; i < NUM_LINE_STYLES; i++)
      {
	if (strcmp (s, _line_styles[i].name) == 0)
	  {
	    _plotter->drawstate->line_type =
	      _line_styles[i].type;
	    _plotter->drawstate->points_are_connected = true;
	    matched = true;
	    break;
	  }
      }
  
  if (matched == false)
    /* don't recognize, silently switch to default mode */
    _g_linemod (_default_drawstate.line_mode);

  /* for future paths, use builtin line style rather than user-specified
     dash array */
  _plotter->drawstate->dash_array_in_effect = false;

  return 0;
}
