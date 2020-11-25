/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_i_openpl (void)
#else
_i_openpl ()
#endif
{
  const char *bg_color_name_s;
  
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* With each call to openpl(), we reset the dynamic GIF-specific data
     members of the GIFPlotter.  The data members, and the values that are
     set, are the same as are used in initializing the GIFPlotter (see
     i_defplot.c). */
     
  _plotter->i_bitmap = (miPixel **)NULL;
  _plotter->i_num_color_indices = 0;
  _plotter->i_bit_depth = 0;
  _plotter->i_frame_nonempty = false;
  _plotter->i_pixels_scanned = 0;
  _plotter->i_pass = 0;
  _plotter->i_hot.x = 0;
  _plotter->i_hot.y = 0;  
  _plotter->i_header_written = false;

  /* invoke generic method, to e.g. create drawing state */
  _g_openpl ();

  /* if there's a user-specified background color, set it in
     device-independent part of drawing state */
  bg_color_name_s = (const char *)_get_plot_param ("BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (bg_color_name_s);

  /* if there's a user-specified transparent color, set it in Plotter */
  {
    const char *transparent_name_s;
    const Colornameinfo *info;

    transparent_name_s = (const char *)_get_plot_param ("TRANSPARENT_COLOR");
    if (transparent_name_s && _string_to_color (transparent_name_s, &info))
      /* have 24-bit RGB */
      {
	_plotter->i_transparent = true;
	_plotter->i_transparent_color.red = info->red;
	_plotter->i_transparent_color.green = info->green;
	_plotter->i_transparent_color.blue = info->blue;
      }
  }

  /* Create new image, consisting of bitmap and colormap; initialized to
     background color.  First entries in color table will be (1)
     transparent color [if there is one, and we're animating] and (2)
     background color.  May be the same. */
  _i_new_image ();
  
  /* frame starts empty */
  _plotter->i_frame_nonempty = false;

  /* GIF file header not yet written */
  _plotter->i_header_written = false;

  return 0;
}

/* Internal function: Create new image, consisting of bitmap and colormap;
   initialized to background color.  First entries in color table will be
   (1) transparent color [if there is one, and we're animating] and (2)
   background color.  Maybe the same. */
void
#ifdef _HAVE_PROTOS
_i_new_image (void)
#else
_i_new_image ()
#endif
{
  int xn, yn, i, j;
  unsigned char bg_color_index;
  
  xn = _plotter->i_xn;
  yn = _plotter->i_yn;

  /* create new image, with bitmap of specified size */
  _plotter->i_bitmap = (miPixel **)_plot_xmalloc (yn * sizeof(miPixel *));
  for (j = 0; j < yn; j++)	/* each row of pixels is contiguous */
    _plotter->i_bitmap[j] = 
      (miPixel *)_plot_xmalloc (xn * sizeof(miPixel));

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
    _i_new_color_index (_plotter->i_transparent_color.red,
			_plotter->i_transparent_color.green,
			_plotter->i_transparent_color.blue);

  /* allocate bg color as next color index in colormap (it could well be
     the same as the transparent index) */
  _i_set_bg_color();
  bg_color_index = _plotter->drawstate->i_bg_color_index;

  /* fill image with background color index */
  for (j = 0; j < yn; j++)	/* each row of pixels is contiguous */
    for (i = 0; i < xn; i++)
      _plotter->i_bitmap[j][i].index = bg_color_index;
}


