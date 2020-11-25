/* This file contains the textangle method, which is a GNu extension to
   libplot.  Together with fontname() and fontsize(), it determines the
   font used for text.  In particular, it sets a drawing attribute: the
   rotation angle, in counterclockwise degrees from the horizontal (in the
   user frame), of labels subsequently drawn on the graphics device.

   Textangle returns the size of the font, in user units.  This may change
   when the rotation angle is changed, since some fonts may not be
   available at all rotation angles, so that a default font must be
   switched to.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_textangle (int angle)
#else
_m_textangle (angle)
     int angle;
#endif
{
  double retval;

  if (!_plotter->open)
    {
      _plotter->error ("textangle: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d\n", 
		 TEXTANGLE, angle);
      else
	{
	  putc (TEXTANGLE, _plotter->outstream);
	  _emit_integer (angle);
	}  
    }

  /* invoke generic method */
  retval = _g_ftextangle ((double)angle);

  return IROUND(retval);
}

double
#ifdef _HAVE_PROTOS
_m_ftextangle (double angle)
#else
_m_ftextangle (angle)
     double angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("ftextangle: invalid operation");
      return -1;
    }

  if (_plotter->outstream == NULL)
    return 0.0;

  if (_plotter->portable_output)
    fprintf (_plotter->outstream, "%c %g\n",
	     TEXTANGLE, angle);
  else
    {
      putc (FTEXTANGLE, _plotter->outstream);
      _emit_float (angle);
    }  

  /* invoke generic method */
  return _g_ftextangle (angle);
}
