/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

int
#ifdef _HAVE_PROTOS
_n_openpl (S___(Plotter *_plotter))
#else
_n_openpl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  const char *bg_color_name_s;
  
  if (_plotter->open)
    {
      _plotter->error (R___(_plotter) "openpl: invalid operation");
      return -1;
    }

  /* invoke generic method, to e.g. create drawing state */
  _g_openpl (S___(_plotter));

  /* if there's a user-specified background color, set it in drawing state */
  bg_color_name_s = (const char *)_get_plot_param (R___(_plotter) "BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (R___(_plotter) bg_color_name_s);

  /* create new pixmap of specified size (all pixels of background color) */
  _n_new_image (S___(_plotter));

  return 0;
}

/* internal function: create new image, consisting of a bitmap; also fill
   with Plotter's background color */
void
#ifdef _HAVE_PROTOS
_n_new_image (S___(Plotter *_plotter))
#else
_n_new_image (S___(_plotter))
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
  _plotter->n_painted_set = (voidptr_t)miNewPaintedSet ();
  _plotter->n_canvas = (voidptr_t)miNewCanvas ((unsigned int)_plotter->n_xn, (unsigned int)_plotter->n_yn, pixel);
}
