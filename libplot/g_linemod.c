/* This file contains the linemod method, which is a standard part of
   libplot.  It sets a drawing attribute: the line mode used in subsequent
   drawing operations. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_linemod (const char *s)
#else
_g_linemod (s)
     const char *s;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("linemod: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath(); /* flush polyline if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _plotter->default_drawstate->line_mode;

  free (_plotter->drawstate->line_mode);
  _plotter->drawstate->line_mode = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (_plotter->drawstate->line_mode, s);

  if (strcmp( s, "disconnected") == 0)
     /* we implement disconnected lines by, in effect, calling point()
	repeatedly */
    {
      _plotter->drawstate->line_type = L_SOLID;
      _plotter->drawstate->points_are_connected = false;
    }
  
  /* The following five line types have long been standard in libplot. */

  else if (strcmp( s, "solid") == 0)
    {
      _plotter->drawstate->line_type = L_SOLID;
      _plotter->drawstate->points_are_connected = true;
    }
  
  else if (strcmp( s, "dotted") == 0)
    {
      _plotter->drawstate->line_type = L_DOTTED;
      _plotter->drawstate->points_are_connected = true;
    }

  else if (strcmp( s, "dotdashed") == 0)
    {
      _plotter->drawstate->line_type = L_DOTDASHED;
      _plotter->drawstate->points_are_connected = true;
    }
  
  else if (strcmp( s, "shortdashed") == 0)
    {
      _plotter->drawstate->line_type = L_SHORTDASHED;
      _plotter->drawstate->points_are_connected = true;
    }

  else if (strcmp( s, "longdashed") == 0)
    {
      _plotter->drawstate->line_type = L_LONGDASHED;
      _plotter->drawstate->points_are_connected = true;
    }
  
  /* N.B. BSD atoplot(1) also recognizes "dotlongdashed", "dotshortdashed",
     and "dotdotdashed" (they omit the final "ed").  Bit patterns unknown.
  
     According to kermit documentation source, the MS-DOS Kermit Tektronix
     emulator has a slightly different ordering for line types
     (cf. linemode.c):

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
    _g_linemod (_plotter->default_drawstate->line_mode);

  return 0;
}
