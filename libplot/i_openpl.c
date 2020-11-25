#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

bool
#ifdef _HAVE_PROTOS
_i_begin_page (S___(Plotter *_plotter))
#else
_i_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* With each call to openpl(), we reset the dynamic GIF-specific data
     members of the GIFPlotter.  The data members, and the values that are
     set, are the same as are used in initializing the GIFPlotter (see
     i_defplot.c). */
     
  _plotter->i_painted_set = (voidptr_t)NULL;
  _plotter->i_canvas = (voidptr_t)NULL;
  _plotter->i_num_color_indices = 0;
  _plotter->i_bit_depth = 0;
  _plotter->i_frame_nonempty = false;
  _plotter->i_pixels_scanned = 0;
  _plotter->i_pass = 0;
  _plotter->i_hot.x = 0;
  _plotter->i_hot.y = 0;  
  _plotter->i_header_written = false;

  /* Create new image, consisting of bitmap and colormap; initialized to
     background color.  First entries in color table will be (1)
     transparent color [if there is one, and we're animating] and (2)
     background color.  May be the same. */
  _i_new_image (S___(_plotter));
  
  /* frame starts empty */
  _plotter->i_frame_nonempty = false;

  /* GIF file header not yet written */
  _plotter->i_header_written = false;

  return true;
}

/* Internal function: Create new image, consisting of bitmap and colormap;
   initialized to background color.  First entries in color table will be
   (1) transparent color [if there is one, and we're animating] and (2)
   background color.  Maybe the same. */
void
#ifdef _HAVE_PROTOS
_i_new_image (S___(Plotter *_plotter))
#else
_i_new_image (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;
  miPixel pixel;
  
  /* colormap starts empty (unused entries initted to `black'; we may later
     need to output some of the unused entries because GIF colormap lengths
     are always powers of 2) */
  _plotter->i_num_color_indices = 0;
  for (i = 0; i < 256; i++)
    {
      _plotter->i_colormap[i].red = 0;
      _plotter->i_colormap[i].green = 0;
      _plotter->i_colormap[i].blue = 0;
    }      

  /* flag any color indices stored in current drawing state as bogus */
  _plotter->drawstate->i_pen_color_status = false;
  _plotter->drawstate->i_fill_color_status = false;
  _plotter->drawstate->i_bg_color_status = false;

  /* Transparency feature of GIF89a files requires that the index of the
     transparent color be the same for all images in the file.  So if we're
     animating, i.e. writing a multi-image file, we allocate the
     transparent color as the first color index (#0) in all images. */
  if (_plotter->i_transparent && _plotter->i_animation)
    /* allocate color cell in colormap; see i_color.c */
    _i_new_color_index (R___(_plotter) 
			_plotter->i_transparent_color.red,
			_plotter->i_transparent_color.green,
			_plotter->i_transparent_color.blue);

  /* allocate bg color as next color index in colormap (it could well be
     the same as the transparent index); also construct a miPixel for it */
  _i_set_bg_color (S___(_plotter));
  pixel.type = MI_PIXEL_INDEX_TYPE;
  pixel.u.index = _plotter->drawstate->i_bg_color_index;

  /* create libxmi miPaintedSet and miCanvas structs */
  _plotter->i_painted_set = (voidptr_t)miNewPaintedSet ();
  _plotter->i_canvas = (voidptr_t)miNewCanvas ((unsigned int)_plotter->i_xn, (unsigned int)_plotter->i_yn, pixel);
}
