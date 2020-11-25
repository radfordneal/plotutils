/* This file contains the linemod routine, which is a standard part of
   libplot.  It sets a drawing attribute: the line mode used in subsequent
   drawing operations. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
linemod (s)
     char *s;
{
  XGCValues gcv;

  /* the canonical four non-solid line patterns */

#define DOTTED_LENGTH 2
#define DOTDASHED_LENGTH 4
#define SHORTDASHED_LENGTH 2
#define LONGDASHED_LENGTH 2
  
  /* these on/off bit patterns are those used by the xterm Tektronix
     emulator, except that the emulator seems incorrectly to have on and
     off interchanged */
  static char dotted[DOTTED_LENGTH]		= { 1, 3 };
  static char dotdashed[DOTDASHED_LENGTH] 	= { 1, 3, 4, 3 };  
  static char shortdashed[SHORTDASHED_LENGTH] 	= { 4, 4 };  
  static char longdashed[LONGDASHED_LENGTH] 	= { 7, 4 };    
  
  /* N.B. ps4014, the Tek->PS translator in Adobe's Transcript package,
     uses { 1, 2 }, { 1, 2, 8, 2 }, { 2, 2 }, { 12, 2 } instead. */

  /* N.B. a genuine Tektronix 4014 (with Enhanced Graphics Module) uses
     { 1, 1 }, { 1, 1, 5, 1 }, { 3, 1 }, { 6, 2 }.  See the Tektronix 4014
     Service Instruction Manual (dated Aug. 1974) for the diode array that
     produces these patterns. */

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linemod() called when graphics device not open\n");
      return -1;
    }

  endpoly();			/* flush polyline if any */

  /* invalid pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.line_mode;

  free (_drawstate->line_mode);
  _drawstate->line_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_drawstate->line_mode, s);

  if (strcmp( s, "disconnected") == 0)
    {
      gcv.line_style = LineSolid; /* in effect we override this */
      _drawstate->points_are_connected = FALSE;
    }

  /* The following five line types have long been standard in the plot
     library. */

  else if (strcmp( s, "solid") == 0)
    {
      gcv.line_style = LineSolid;
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "dotted") == 0)
    {
      XSetDashes (_xdata.dpy, _drawstate->gc, 0, dotted, DOTTED_LENGTH);
      gcv.line_style = LineOnOffDash;
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "dotdashed") == 0)
    {
      XSetDashes (_xdata.dpy, _drawstate->gc, 0, dotdashed, DOTDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "shortdashed") == 0)
    {
      XSetDashes (_xdata.dpy, _drawstate->gc, 0, shortdashed, SHORTDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      _drawstate->points_are_connected = TRUE;
    }
  else if (strcmp( s, "longdashed") == 0)
    {
      XSetDashes (_xdata.dpy, _drawstate->gc, 0, longdashed, LONGDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      _drawstate->points_are_connected = TRUE;
    }
  else
    {
      /* don't recognize, silently switch to default mode */
      linemod (_default_drawstate.line_mode);
      return 0;
    }
  
  XChangeGC (_xdata.dpy, _drawstate->gc, GCLineStyle, &gcv);

  _handle_x_events();

  return 0;
}
