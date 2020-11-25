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
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_textangle (R___(Plotter *_plotter) int angle)
#else
_m_textangle (R___(_plotter) angle)
     S___(Plotter *_plotter;) 
     int angle;
#endif
{
  double retval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "textangle: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_TEXTANGLE);
  _meta_emit_integer (R___(_plotter) angle);
  _meta_emit_terminator (S___(_plotter));

  /* invoke generic method */
  retval = _g_ftextangle (R___(_plotter) (double)angle);

  return IROUND(retval);
}

double
#ifdef _HAVE_PROTOS
_m_ftextangle (R___(Plotter *_plotter) double angle)
#else
_m_ftextangle (R___(_plotter) angle)
     S___(Plotter *_plotter;) 
     double angle;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "ftextangle: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_TEXTANGLE : (int)O_FTEXTANGLE);
  _meta_emit_float (R___(_plotter) angle);
  _meta_emit_terminator (S___(_plotter));

  /* invoke generic method */
  return _g_ftextangle (R___(_plotter) angle);
}
