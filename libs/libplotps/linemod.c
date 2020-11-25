/* This file contains the linemod routine, which is a standard part of
   libplot.  It sets a drawing attribute: the line mode used in subsequent
   drawing operations. */

#include "sys-defines.h" 
#include "plot.h" 
#include "extern.h"

/* Low-level data on line types, for idraw and PS.  These arrays are
   indexed into by the drawing state variable _drawstate->ps_line_type. */

/* 16-bit brush patterns for idraw (1 = on, 0 = off): */
long _ps_line_type_bit_vector[] = 
/* solid, dotted, dotdashed, shortdashed, longdashed */
{ 0xffff, 0x8888, 0xc3f0, 0xf0f0, 0xffc0 };

/* corresponding dash arrays for PS (cylically used, on/off/on/off... */
char *_ps_line_type_setdash[]  =
/* these on/off bit patterns are those used by libplotX, and also by the
   xterm Tektronix emulator, except that the emulator seems incorrectly to
   have on and off interchanged */
{ "", "1 3", "1 3 4 3", "4 4", "7 4" };

int
linemod (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linemod() called when graphics device not open\n");
      return -1;
    }

  endpoly();			/* flush polyline if any */

  /* invalid pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.line_mode;

  if (strcmp( s, "disconnected") == 0)
   {
     /* we now implement disconnected lines by, in effect, calling point()
	repeatedly */
      _drawstate->points_are_connected = FALSE;
      _drawstate->ps_line_type = L_SOLID;
    }

  /* The following five line types have long been standard in the plot
     library. */

  else if (strcmp( s, "solid") == 0)
    {
      _drawstate->points_are_connected = TRUE;
      _drawstate->ps_line_type = L_SOLID;
    }
  else if (strcmp( s, "dotted") == 0)
    {
      _drawstate->points_are_connected = TRUE;
      _drawstate->ps_line_type = L_DOTTED;
    }
  else if (strcmp( s, "dotdashed") == 0)
    {
      _drawstate->points_are_connected = TRUE;
      _drawstate->ps_line_type = L_DOTDASHED;
    }
  else if (strcmp( s, "shortdashed") == 0)
    {
      _drawstate->points_are_connected = TRUE;
      _drawstate->ps_line_type = L_SHORTDASHED;
    }
  else if (strcmp( s, "longdashed") == 0)
    {
      _drawstate->points_are_connected = TRUE;
      _drawstate->ps_line_type = L_LONGDASHED;
    }
  else
    /* don't recognize, silently switch to default mode */
    {
      linemod (_default_drawstate.line_mode);
      return 0;
    }

  free (_drawstate->line_mode);
  _drawstate->line_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_drawstate->line_mode, s);

  return 0;
}
