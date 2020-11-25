/* This file contains the fontsize routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the size of the font used for
   text subsequently drawn on the graphics device.  The selected font may
   also be changed by calling the fontname or textangle routines.

   The argument to fontsize() is the size in user units.  The fontsize is
   normally taken to be a minimum acceptable vertical spacing between
   adjacent lines of text.  If a font size is not available, the closest
   available size will be substituted.

   The fontsize, fontname, and textangle routines return the fontsize in
   user units, as an aid to vertical positioning by the user.  The return
   value is zero if no information on font size is available. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
ffontsize (size)
     double size;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ffontsize() called when graphics device not open\n");
      return -1;
    }

  if (size < 0.0)		/* invalid size resets to default */
    size = _default_drawstate.font_size;
  
  if (_drawstate->font_size != size)
    /* font change */
    {
      /* set the new nominal size in the drawing state */
      _drawstate->font_size = size;
      /* if an X font, retrieve; update fields in the _drawstate structure */
      _retrieve_font();
    }
  
  _handle_x_events();

  return _drawstate->x_font_size;
}
