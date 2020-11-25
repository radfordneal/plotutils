/* This file contains the textangle routine, which is a GNu extension to
   libplot.  Together with fontname() and fontsize(), it determines the
   font used for text.  In particular, it sets a drawing attribute: the
   rotation angle, in counterclockwise degrees from the horizontal (in the
   user frame), of labels subsequently drawn on the graphics device.

   Textangle returns the size of the font, in user units.  This may change
   when the rotation angle is changed, since some fonts may not be
   available at all rotation angles, so that a default font must be
   switched to.  The return value is zero if no information on font size is
   available.

   In this version (for raw libplot) we return zero. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
textangle (angle)
     int angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: textangle() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d\n", 
	     TEXTANGLE, angle);
  else
    {
      putc (TEXTANGLE, _outstream);
      _putshort (angle, _outstream);
    }  

  return 0;
}

double
ftextangle (angle)
     double angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ftextangle() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g\n",
	     FTEXTANGLE, angle);
  else
    {
      putc (FTEXTANGLE, _outstream);
      _putfloat (angle, _outstream);
    }  

  return 0.0;
}
