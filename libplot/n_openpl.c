/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_n_openpl (void)
#else
_n_openpl ()
#endif
{
  const char *bg_color_name_s;
  
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* invoke generic method, to e.g. create drawing state */
  _g_openpl ();

  /* if there's a user-specified background color, set it in drawing state */
  bg_color_name_s = (const char *)_get_plot_param ("BG_COLOR");
  if (bg_color_name_s)
    _plotter->bgcolorname (bg_color_name_s);

  /* create new pixmap of specified size (all pixels of background color) */
  _n_new_image ();

  return 0;
}

/* internal function: create new image, consisting of a bitmap; also fill
   with Plotter's background color */
void
#ifdef _HAVE_PROTOS
_n_new_image (void)
#else
_n_new_image ()
#endif
{
  int i, j, xn, yn;
  unsigned char red, green, blue;

  /* compute 24-bit color */
  red = ((unsigned int)(_plotter->drawstate->bgcolor.red) >> 8) & 0xff;
  green = ((unsigned int)(_plotter->drawstate->bgcolor.green) >> 8) & 0xff;
  blue = ((unsigned int)(_plotter->drawstate->bgcolor.blue) >> 8) & 0xff;  

  xn = _plotter->n_xn;
  yn = _plotter->n_yn;
  _plotter->n_bitmap = (miPixel **)_plot_xmalloc (yn * sizeof(miPixel *));
  for (j = 0; j < yn; j++)	/* each row of pixels is contiguous */
    {
      _plotter->n_bitmap[j] = 
	(miPixel *)_plot_xmalloc (xn * sizeof(miPixel));
      for (i = 0; i < xn; i++)
	{
	  _plotter->n_bitmap[j][i].rgb[0] = red;
	  _plotter->n_bitmap[j][i].rgb[1] = green;
	  _plotter->n_bitmap[j][i].rgb[2] = blue;
	}
    }
}
