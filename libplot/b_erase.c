#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_b_erase_page (S___(Plotter *_plotter))
#else
_b_erase_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* delete image, i.e., deallocate libxmi canvas */
  _b_delete_image (S___(_plotter));

  /* create new image, i.e. libxmi canvas, initialized to background color */
  _b_new_image (S___(_plotter));

  return true;
}
