/* This file contains the fontsize method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the size of the font used for
   text subsequently drawn on the graphics device.  The selected font may
   also be changed by invoking the fontname method.

   The argument to fontsize() is the size in printer's points.  If a font
   size is not available, the closest available size will be substituted.

   The fontsize and fontname routines return the fontsize in user units, as
   an aid to vertical positioning by the user.  (The fontsize is normally
   taken to be an acceptable vertical spacing between adjacent lines of
   text.)  A return value of zero means that no information on font size is
   available.

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence on the arguments given to the
   space() routine.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_fontsize (int size)
#else
_m_fontsize (size)
    int size;
#endif
{
  double retval;

  if (!_plotter->open)
    {
      _plotter->error ("fontsize: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d\n", 
		 FONTSIZE, size);
      else
	{
	  putc (FONTSIZE, _plotter->outstream);
	  _emit_integer (size);
	}
    }
  
  /* invoke generic method to retrieve font, set font size in drawing state
     so that labelwidth() computations will work */
  retval = _g_ffontsize ((double)size);

  return IROUND(retval);
}

double
#ifdef _HAVE_PROTOS
_m_ffontsize (double size)
#else
_m_ffontsize (size)
    double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("ffontsize: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g\n", 
		 FONTSIZE, size);
      else
	{
	  putc (FFONTSIZE, _plotter->outstream);
	  _emit_float (size);
	}
    }
  
  /* invoke generic method to retrieve font, set font size in drawing state
     so that labelwidth() computations will work */
  return _g_ffontsize (size);
}
