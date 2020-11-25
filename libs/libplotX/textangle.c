/* This file contains the textangle routine, which is a GNU extension to
   libplot.  Together with fontname() and fontsize(), it determines the
   font used for text.  In particular, it sets a drawing attribute: the
   rotation angle, in counterclockwise degrees from the horizontal (in the
   user frame), of labels subsequently drawn on the graphics device.

   Textangle returns the size of the font, in user units.  This may change
   when the rotation angle is changed, since some fonts may not be
   available at all rotation angles, so that a default font must be
   switched to.  The return value is zero if no information on font size is
   available. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
ftextangle (angle)
     double angle;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ftextangle() called when graphics device not open\n");
      return -1;
    }

  if (angle != _drawstate->text_rotation) /* font change */
    {
      /* save new rotation angle */
      _drawstate->text_rotation = angle;

      /* if an X font, retrieve; update fields in the _drawstate structure */
      _retrieve_font();
    }  
  
  _handle_x_events();

  /* return value is size in user units */
  return _drawstate->x_font_size;
}
