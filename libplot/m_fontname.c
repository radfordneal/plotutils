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
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_m_ffontname (R___(Plotter *_plotter) const char *s)
#else
_m_ffontname (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  double size;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontname: invalid operation");
      return -1;
    }

  /* invoke generic method to retrieve font into the drawing state */
  size = _g_ffontname (R___(_plotter) s);

  _meta_emit_byte (R___(_plotter) (int)O_FONTNAME);
  _meta_emit_string (R___(_plotter) _plotter->drawstate->font_name);

  return size;
}

int 
#ifdef _HAVE_PROTOS
_m_fontname (R___(Plotter *_plotter) const char *s)
#else
_m_fontname (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  double new_size = _plotter->ffontname (R___(_plotter) s);
  
  return IROUND(new_size);
}
