/* This file contains the bgcolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the background color.

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_bgcolor (int red, int green, int blue)
#else
_m_bgcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("bgcolor: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_BGCOLOR);
  _meta_emit_integer (red);
  _meta_emit_integer (green);
  _meta_emit_integer (blue);
  _meta_emit_terminator ();
  
  /* invoke generic method */
  return _g_bgcolor (red, green, blue);
}
