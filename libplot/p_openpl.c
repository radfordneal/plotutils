/* Note: a PS Plotter emits graphics only after all pages of graphics have
   been drawn, and the Plotter is deleted.  This delayed output allows the
   output to satisfy the DSC (Document Structuring Conventions). */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_p_begin_page (S___(Plotter *_plotter))
#else
_p_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;

  /* initialize `font used' array(s) for this page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;
#ifdef USE_LJ_FONTS_IN_PS
  for (i = 0; i < NUM_PCL_FONTS; i++)
    _plotter->data->page->pcl_font_used[i] = false;
#endif

  return true;
}
