/* This file contains the fontname method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the name of the font used for
   text subsequent drawn on the graphics device.  The selected font may
   also be changed by calling the fontname routine.

   The fontsize and fontname methods return the fontsize in user units, as
   an aid to vertical positioning by the user.  (The fontsize is normally
   taken to be an acceptable vertical spacing between adjacent lines of
   text.)  A return value of zero means that no information on font size is
   available.

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence on the arguments given to the
   space() method. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_m_ffontname (const char *s)
#else
_m_ffontname (s)
     const char *s;
#endif
{
  double size;

  if (!_plotter->open)
    {
      _plotter->error ("ffontname: invalid operation");
      return -1;
    }

  /* invoke generic method to retrieve font into the drawing state */
  size = _g_ffontname (s);

  if (_plotter->outstream)
    fprintf (_plotter->outstream, "%c%s\n", 
	     (int)O_FONTNAME, _plotter->drawstate->font_name);

  return size;
}

int 
#ifdef _HAVE_PROTOS
_m_fontname (const char *s)
#else
_m_fontname (s)
     const char *s;
#endif
{
  double new_size = _plotter->ffontname (s);
  
  return IROUND(new_size);
}
