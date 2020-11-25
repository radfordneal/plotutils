/* This file contains the low-level _putshort routine, which takes an
   integer argument and writes it, either in traditional plot format or in
   system format, to a specified output stream.  It is called by functions
   in raw libplot.

   In traditional plot format we assume that two bytes suffice to represent
   any integer; this confines us to the range from -0x7fff to 0x7fff [or
   from -0x10000 to 0x7fff, on a two's complement machine].  We write out
   the two bytes in an order specified by the external variable
   _libplot_output_high_byte_first.

   This file also contains the _putfloat routine, used by the new (floating
   point) routines in raw libplot. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
_putshort (x, stream)
     int x;
     FILE *stream;
{
  switch (_libplot_output_high_byte_first)
    {
    case 1:
      putc ((x >> 8) & 0xff, stream);
      putc (x & 0xff, stream);
      break;
    case -1:
      putc (x & 0xff, stream);
      putc ((x >> 8) & 0xff, stream);
      break;
    default:
    case 0:
      /* use system default, though these days a short may not be 2 bytes */
      {
	short i = x;		/* keep this for a while, for compatibility */

	fwrite (&i, sizeof(i), 1, stream);
      }
      break;
    }
}

/* must do better than the below! */

void
_putfloat (x, stream)
     double x;
     FILE *stream;
{
  float f = x;

  fwrite (&f, sizeof(float), 1, stream);
}
