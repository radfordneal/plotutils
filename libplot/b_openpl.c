#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

bool
#ifdef _HAVE_PROTOS
_b_begin_page (S___(Plotter *_plotter))
#else
_b_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* create new pixmap of specified size (all pixels of background color) */
  _b_new_image (S___(_plotter));

  return true;
}

/* internal function: create new image, consisting of a bitmap; also fill
   with Plotter's background color */
void
#ifdef _HAVE_PROTOS
_b_new_image (S___(Plotter *_plotter))
#else
_b_new_image (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  unsigned char red, green, blue;
  miPixel pixel;

  /* compute 24-bit bg color, and construct a miPixel for it */
  red = ((unsigned int)(_plotter->drawstate->bgcolor.red) >> 8) & 0xff;
  green = ((unsigned int)(_plotter->drawstate->bgcolor.green) >> 8) & 0xff;
  blue = ((unsigned int)(_plotter->drawstate->bgcolor.blue) >> 8) & 0xff;  
  pixel.type = MI_PIXEL_RGB_TYPE;
  pixel.u.rgb[0] = red;
  pixel.u.rgb[1] = green;
  pixel.u.rgb[2] = blue;

  /* create libxmi miPaintedSet and miCanvas structs */
  _plotter->b_painted_set = (voidptr_t)miNewPaintedSet ();
  _plotter->b_canvas = (voidptr_t)miNewCanvas ((unsigned int)_plotter->b_xn, (unsigned int)_plotter->b_yn, pixel);
}
