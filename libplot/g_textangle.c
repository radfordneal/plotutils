/* This file contains the textangle method, which is a GNU extension to
   libplot.  Together with fontname() and fontsize(), it determines the
   font used for text.  In particular, it sets a drawing attribute: the
   rotation angle, in counterclockwise degrees from the horizontal (in the
   user frame), of labels subsequently drawn on the graphics device.

   Textangle returns the size of the font, in user units.  This may change
   when the rotation angle is changed, since some fonts may not be
   available at all rotation angles, so that a default font must be
   switched to. */

/* This generic version simply stores the new text angle in the current
   drawing state. */

#include "sys-defines.h"
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_g_ftextangle (R___(Plotter *_plotter) double angle)
#else
_g_ftextangle (R___(_plotter) angle)
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

  /* save new rotation angle */
  _plotter->drawstate->text_rotation = angle;
  
  /* compute true size (may be quantized) */
  _plotter->retrieve_font (S___(_plotter));
  
  /* return quantized user-specified font size */
  return _plotter->drawstate->true_font_size;
}
