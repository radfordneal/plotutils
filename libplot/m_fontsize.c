/* This file contains the fontsize method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the size of the font used for
   text subsequently drawn on the graphics device.  The selected font may
   also be changed by invoking the fontname or textangle methods.

   The argument to fontsize() is the size in user units.  The fontsize is
   normally taken to be a minimum acceptable vertical spacing between
   adjacent lines of text.  If a font size is not available, the closest
   available size will be substituted.

   The fontsize, fontname, and textangle methods return the fontsize in
   user units, as an aid to vertical positioning by the user.  The return
   value is zero if no information on font size is available. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_fontsize (R___(Plotter *_plotter) int size)
#else
_m_fontsize (R___(_plotter) size)
     S___(Plotter *_plotter;) 
     int size;
#endif
{
  double retval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fontsize: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_FONTSIZE);
  _meta_emit_integer (R___(_plotter) size);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method to retrieve font, set font size in drawing state
     so that labelwidth() computations will work */
  retval = _g_ffontsize (R___(_plotter) (double)size);

  return IROUND(retval);
}

double
#ifdef _HAVE_PROTOS
_m_ffontsize (R___(Plotter *_plotter) double size)
#else
_m_ffontsize (R___(_plotter) size)
     S___(Plotter *_plotter;) 
     double size;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontsize: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_FONTSIZE : (int)O_FFONTSIZE);
  _meta_emit_float (R___(_plotter) size);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method to retrieve font, set font size in drawing state
     so that labelwidth() computations will work */
  return _g_ffontsize (R___(_plotter) size);
}
