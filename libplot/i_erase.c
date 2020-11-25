/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_i_erase (S___(Plotter *_plotter))
#else
_i_erase (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "erase: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* If we're animating, emit the GIF header, and emit the just-finished
     frame as one of the images in the animated GIF.  But don't do this
     for the zeroth frame unless it's nonempty. */
  if (_plotter->i_animation && _plotter->page_number == 1 && _plotter->outfp
      && (_plotter->frame_number > 0 || _plotter->i_frame_nonempty))
    {
      if (_plotter->i_header_written == false)
	{
	  _i_write_gif_header (S___(_plotter));
	  _plotter->i_header_written = true;
	}
      /* emit image using RLE module (see i_rle.c) */
      _i_write_gif_image (S___(_plotter));
    }

  /* delete image: deallocate frame's libxmi canvas, reset frame's color
     table */
  _i_delete_image (S___(_plotter));

  /* Create new image, consisting of libxmi canvas and colormap;
     initialized to background color.  First entries in the color table
     will be (1) transparent color [if there is one, and we're animating]
     and (2) background color. */
  _i_new_image (S___(_plotter));
  
  /* on to next frame */
  _plotter->frame_number++;

  /* frame starts empty */
  _plotter->i_frame_nonempty = false;

  return 0;
}
