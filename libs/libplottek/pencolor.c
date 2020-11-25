/* This file contains the pencolor routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the pen color (``foreground
   color'') of objects created in the drawing operations that follow.  The
   fill color may be set separately, by calling fillcolor() and fill().

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

/* In libplottek, color() has no effect unless the Tektronix emulation of
   MS-DOS kermit is being used.  If so, we compute a quantized color and
   output the appropriate ANSI escape sequence. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
pencolor (red, green, blue)
     int red, green, blue;
{
  int new_kermit_fgcolor;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: pencolor() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.fgcolor.red;
      green = _default_drawstate.fgcolor.green;
      blue = _default_drawstate.fgcolor.blue;
    }

  /* save our notion of foreground color */
  (_drawstate->fgcolor).red = red;
  (_drawstate->fgcolor).green = green;  
  (_drawstate->fgcolor).blue = blue;

  if (_tek_display_type == D_KERMIT)
    {
      new_kermit_fgcolor = _kermit_pseudocolor (red, green, blue);
      /* swap white and black pen colors, since kermit Tektronix emulator
	 has a black background */
      if (new_kermit_fgcolor == ANSI_SYS_BLACK)
	new_kermit_fgcolor = ANSI_SYS_WHITE;
      else if (new_kermit_fgcolor == ANSI_SYS_WHITE)
	new_kermit_fgcolor = ANSI_SYS_BLACK;
      if (new_kermit_fgcolor != _drawstate->kermit_fgcolor)
	{
	  fprintf (_outstream,
		   _kermit_fgcolor_escapes[new_kermit_fgcolor]);
	  _drawstate->kermit_fgcolor = new_kermit_fgcolor;
	}
    }
  
  return 0;
}
