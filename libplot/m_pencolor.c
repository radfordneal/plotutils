/* This file contains the pencolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the pen color (``foreground
   color'') of objects created in the drawing operations that follow.  The
   fill color may be set separately, by invoking fillcolor() and
   filltype().

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_pencolor (int red, int green, int blue)
#else
_m_pencolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("pencolor: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_PENCOLOR);
  _meta_emit_integer (red);
  _meta_emit_integer (green);
  _meta_emit_integer (blue);
  _meta_emit_terminator ();
  
  /* invoke generic method */
  return _g_pencolor (red, green, blue);
}
