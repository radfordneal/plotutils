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

/* This generic version simply stores the new font size in the current
   drawing state. */

#include "sys-defines.h"
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_g_ffontsize (R___(Plotter *_plotter) double size)
#else
_g_ffontsize (R___(_plotter) size)
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

  if (size < 0.0)		/* reset to default */
    size = _plotter->drawstate->default_font_size;

  /* set the new nominal size in the drawing state */
  _plotter->drawstate->font_size = size;

  /* compute true size (may be quantized) */
  _plotter->retrieve_font (S___(_plotter));
  
  /* return quantized user-specified font size */
  return _plotter->drawstate->true_font_size;
}
