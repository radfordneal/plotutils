/* This file contains the fontsize routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the size of the font used for
   text subsequently drawn on the graphics device.  The selected font may
   also be changed by calling the fontname routine.

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

   /* In raw libplot, we return 0, since we have no information about the
   mapping between user coordinates and device coordinates that will
   ultimately be performed. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
fontsize (size)
    int size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fontsize() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d\n", 
	     FONTSIZE, size);
  else
    {
      putc (FONTSIZE, _outstream);
      _putshort (size, _outstream);
    }
  
  return 0;
}

double
ffontsize (size)
    double size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fontsize() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %f\n", 
	     FFONTSIZE, size);
  else
    {
      putc (FFONTSIZE, _outstream);
      _putfloat (size, _outstream);
    }
  
  return 0.0;
}
