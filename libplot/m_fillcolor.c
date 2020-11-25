/* This file contains the fillcolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the fill color of objects created
   in the following drawing operations.  Actually the true fill color (if
   filling is not disabled) will be a desaturated version of the
   user-specified fill color.  The desaturation level is set by invoking
   filltype().

   In principle we support 48-bit color (16 bits, i.e. 0x0000 through
   0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_fillcolor (int red, int green, int blue)
#else
_m_fillcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fillcolor: invalid operation");
      return -1;
    }

  _meta_emit_byte ((int)O_FILLCOLOR);
  _meta_emit_integer (red);
  _meta_emit_integer (green);
  _meta_emit_integer (blue);
  _meta_emit_terminator ();

  /* invoke generic method */
  return _g_fillcolor (red, green, blue);
}
