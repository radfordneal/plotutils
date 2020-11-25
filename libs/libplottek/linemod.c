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
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linemod() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  /* invalid pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.line_mode;

  free (_drawstate->line_mode);
  _drawstate->line_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_drawstate->line_mode, s);

  if (strcmp( s, "disconnected") == 0)
    _drawstate->points_are_connected = FALSE; /* tek_line_type will be ignored */
  
  /* Following five line types should be recognized by any Tektronix
     (emulator). They have long been standard in the plot library. */

  else if (strcmp( s, "solid") == 0)
    {
      _drawstate->tek_line_type = L_SOLID;
      _drawstate->points_are_connected = TRUE;
    }
  
  else if (strcmp( s, "dotted") == 0)
    {
      _drawstate->tek_line_type = L_DOTTED;
      _drawstate->points_are_connected = TRUE;
    }

  else if (strcmp( s, "dotdashed") == 0)
    {
      _drawstate->tek_line_type = L_DOTDASHED;
      _drawstate->points_are_connected = TRUE;
    }
  
  else if (strcmp( s, "shortdashed") == 0)
    {
      _drawstate->tek_line_type = L_SHORTDASHED;
      _drawstate->points_are_connected = TRUE;
    }

  else if (strcmp( s, "longdashed") == 0)
    {
      _drawstate->tek_line_type = L_LONGDASHED;
      _drawstate->points_are_connected = TRUE;
    }
  
  /* N.B. BSD atoplot(1) also recognizes "dotlongdashed", "dotshortdashed",
     and "dotdotdashed" (they omit the final "ed").  Bit patterns unknown.
  
     According to kermit documentation source, the MS-DOS Kermit Tektronix
     emulator has a slightly different ordering for line types
     (cf. tek_linemode.c):

        ` = solid		11111111 11111111
        a = dotted      	10101010 10101010 
        b = shortdashed		11110000 11110000
        c = dotdashed           11111010 11111010
        d = dotlongdashed       11111111 11001100
        e = dotdotdashed        11111100 10010010
        x = user defined (by ESC / Pn a)
        y = user defined (by ESC / Pn b)
        z = user defined (by ESC / Pn c)

     The first character is the line type character recognized by 
     the emulator; see tek_linemod.c.  Also, linetypes for VT-type terminals 
     in tektronix emulator mode allegedly include:

        ` = solid
        a = dotted
	b = shortdashed
	c = dotdashed
	d = dotlongdashed
	h = solid (bold) 
        i = dotted (bold) 
        j = shortdashed (bold)
        k = dotdashed (bold)
        l = dotlongdashed (bold)

    If only... */	  

  else
    /* don't recognize, silently switch to default mode */
    linemod (_default_drawstate.line_mode);

  return 0;
}
