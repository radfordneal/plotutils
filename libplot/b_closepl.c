#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

bool
#ifdef _HAVE_PROTOS
_b_end_page (S___(Plotter *_plotter))
#else
_b_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int retval;

  /* Possibly output the page's bitmap.  In the base BitmapPlotter class
     this is a no-op (see below), but it may do something in derived
     classes. */
  retval = _maybe_output_image (S___(_plotter));

  /* tear down */
  _b_delete_image (S___(_plotter));

  return (retval < 0 ? false : true);
}

/* tear down image, i.e. deallocate libxmi canvas */
void
#ifdef _HAVE_PROTOS
_b_delete_image (S___(Plotter *_plotter))
#else
_b_delete_image (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* deallocate libxmi's drawing canvas (and painted set struct too) */
  miDeleteCanvas ((miCanvas *)_plotter->b_canvas);
  _plotter->b_canvas = (voidptr_t)NULL;
  miDeletePaintedSet ((miPaintedSet *)_plotter->b_painted_set);
  _plotter->b_painted_set = (voidptr_t)NULL;
}

/* This is the BitmapPlotter-specific version of the _maybe_output_image()
   method, which is invoked when a page is finished.  It's a no-op; in
   derived classes such as the PNMPlotter and PNGPlotter classes, it's
   overridden by a version that actually does something.  */

int
#ifdef _HAVE_PROTOS
_b_maybe_output_image (S___(Plotter *_plotter))
#else
_b_maybe_output_image (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return 0;
}  

