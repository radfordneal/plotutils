/* A PSPlotter simply resets the output buffer, discarding all objects
   written to it, resets the bounding box, and marks all Postscript fonts
   as unused on the current page. */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_p_erase_page (S___(Plotter *_plotter))
#else
_p_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;

  /* reinitialize `font used' array(s) for this page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;
#ifdef USE_LJ_FONTS_IN_PS
  for (i = 0; i < NUM_PCL_FONTS; i++)
    _plotter->data->page->pcl_font_used[i] = false;
#endif

  return true;
}
