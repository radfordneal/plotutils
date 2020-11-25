/* This file contains the bgcolor method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the background color.

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

#include "sys-defines.h"
#include "plot.h"
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

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d %d %d\n", 
		 (int)O_BGCOLOR, red, green, blue);
      else
	{
	  putc ((int)O_BGCOLOR, _plotter->outstream);
	  _emit_integer (red);
	  _emit_integer (green);
	  _emit_integer (blue);
	}
    }
  
  /* invoke generic method */
  return _g_bgcolor (red, green, blue);
}
