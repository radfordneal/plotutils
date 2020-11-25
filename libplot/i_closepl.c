/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

#include "sys-defines.h"
#include "extern.h"
#include "i_rle.h"		/* use miGIF RLE (non-LZW) compression */
#include "xmi.h"

/* GIF89a frame disposal methods (a 3-bit field; values 4..7 are reserved) */
#define DISP_UNSPECIFIED 0
#define DISP_NONE 1
#define DISP_RESTORE_TO_BACKGROUND 2
#define DISP_RESTORE_TO_PREVIOUS 3

/* forward references */
static bool _same_colormap ____P((plColor cmap1[256], plColor cmap2[256], int num1, int num2));

int
#ifdef _HAVE_PROTOS
_i_closepl(S___(Plotter *_plotter))
#else
_i_closepl(S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int retval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* Output current frame as a GIF image, preceded by a GIF header if
     necessary.  This applies only if this is page #1. */
#ifdef LIBPLOTTER
  if (_plotter->outfp || _plotter->outstream)
#else
  if (_plotter->outfp)
#endif
    /* have an output stream */
    {
      if (_plotter->page_number == 1)
	{
	  if (_plotter->i_header_written == false)
	    {
	      _i_write_gif_header (S___(_plotter));
	      _plotter->i_header_written = true;
	    }
	  /* emit GIF image of current frame using RLE module (see i_rle.c) */
	  _i_write_gif_image (S___(_plotter));
	  _i_write_gif_trailer (S___(_plotter));
	}
    }
  
  /* delete image: deallocate frame's canvas, reset frame's color table */
  _i_delete_image (S___(_plotter));

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate (S___(_plotter));
    }
  
  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  /* attempt to flush (will test whether stream is jammed) */
  retval = _plotter->flushpl (S___(_plotter));

  _plotter->open = false;	/* flag device as closed */

  return retval;
}

void
#ifdef _HAVE_PROTOS
_i_write_gif_header (S___(Plotter *_plotter))
#else
_i_write_gif_header (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i, packed_bits;

  /* determine whether transparency extension is really needed */
  if (_plotter->i_transparent)
    {
      if (_plotter->i_animation)
	/* transparent color index will be #0 in each image's color table;
	   see i_color.c */
	{
	  _plotter->i_transparent = true;
	  _plotter->i_transparent_index = 0;
	}
      else			/* only 1 image, and 1 color table */
	{
	  bool found = false;
	  plColor t_color;

	  t_color = _plotter->i_transparent_color;
	  /* search for user-specified color */
	  for (i = 0; i < _plotter->i_num_color_indices; i++)
	    {
	      if (_plotter->i_colormap[i].red == t_color.red
		  && _plotter->i_colormap[i].green == t_color.green
		  && _plotter->i_colormap[i].blue == t_color.blue)
		{
		  found = true;
		  break;
		}
	    }
	  if (found)
	    {
	      _plotter->i_transparent = true;
	      _plotter->i_transparent_index = i;
	    }
	  else			/* transparency not needed */
	    _plotter->i_transparent = false;
	}
    }

  /* Header block, including Signature and Version. */

  /* To express transparency, a nontrivial number of iterations, or a
     nontrivial delay between successive images, need GIF89a format, not
     GIF87a. */
  if (_plotter->i_transparent 
      || (_plotter->i_animation && _plotter->i_iterations > 0)
      || (_plotter->i_animation && _plotter->i_delay > 0))
    _plotter->write_string (R___(_plotter) "GIF89a");
  else
    _plotter->write_string (R___(_plotter) "GIF87a");

  /* Logical Screen Descriptor Block */

  /* Logical Screen Width and Height (2-byte unsigned ints) */
  _i_write_short_int (R___(_plotter) (unsigned int)_plotter->i_xn);
  _i_write_short_int (R___(_plotter) (unsigned int)_plotter->i_yn);

  /* Global Color Table Flag (1 bit) [1/0 = global table follows / doesn't
     follow].  Represented by 0x80 / 0x00 respectively. */
  packed_bits = 0x80;

  /* Color Resolution, i.e. bitdepth minus 1, with min=0 (3 bits) */
  packed_bits |= (IMAX(_plotter->i_bit_depth - 1, 0)) << 4;

  /* Sort Flag [0 = unordered] (1 bit) */

  /* Size of Global Color Table, i.e. bitdepth minus 1, with min=0 (3 bits) */
  packed_bits |= (IMAX(_plotter->i_bit_depth - 1, 0));

  /* write 1 byte of packed bits */
  _plotter->write_byte (R___(_plotter) (unsigned char)packed_bits);

  /* Background Color Index (if there's no global color table this field
     should be set to 0)  */
  _plotter->write_byte (R___(_plotter) _plotter->drawstate->i_bg_color_index);

  /* Pixel Aspect Ratio (0 = field unused) */
  _plotter->write_byte (R___(_plotter) (unsigned char)0);

  /* Global Color Table (expanded to next higher power of 2, with min=2) */
  for (i = 0; i < (1 << IMAX(_plotter->i_bit_depth, 1)); ++i)
    {
      _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_colormap[i].red);
      _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_colormap[i].green);
      _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_colormap[i].blue);
      /* stash table (for comparison with color tables of later frames) */
      _plotter->i_global_colormap[i] = _plotter->i_colormap[i];
    }
  _plotter->i_num_global_color_indices = _plotter->i_num_color_indices;

  /* Netscape Loop Extension Block (extension blocks are a GIF89a feature;
     this requests `looping' of subsequent images) */

  if (_plotter->i_animation && _plotter->i_iterations > 0)
    {
      /* Extension Introducer */
      _plotter->write_byte (R___(_plotter) (unsigned char)'!');

      /* Application Extension Label */
      _plotter->write_byte (R___(_plotter) (unsigned char)0xff);

      /* Block Size (fixed at 11) */
      _plotter->write_byte (R___(_plotter) (unsigned char)11);
      /* Application Identifier (8 bytes) and Auth. Code (3 bytes) */
      _plotter->write_string (R___(_plotter) "NETSCAPE2.0");

      /* Block Size (fixed at 3) */
      _plotter->write_byte (R___(_plotter) (unsigned char)0x03);
      /* Block, 3 bytes long */
      _plotter->write_byte (R___(_plotter) (unsigned char)0x01);/* what is this? */
      _i_write_short_int (R___(_plotter) (unsigned int)(_plotter->i_iterations));

      /* Block Terminator (0-length data block) */
      _plotter->write_byte (R___(_plotter) (unsigned char)0x00);
    }
}

/* Write image descriptor, including color table.  Also scan image, and
   compress and write the resulting stream of color indices. */
void
#ifdef _HAVE_PROTOS
_i_write_gif_image (S___(Plotter *_plotter))
#else
_i_write_gif_image (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool write_local_table;
  int i, min_code_size, packed_bits;

  /* Graphic Control Block (a GIF89a feature; modifies following image
     descriptor).  Needed to express transparency of each image, or a
     non-default delay after each image. */
  if (_plotter->i_transparent 
      || (_plotter->i_animation && _plotter->i_delay > 0))
    {
      unsigned char packed_byte;

      /* Extension Introducer */
      _plotter->write_byte (R___(_plotter) (unsigned char)'!');

      /* Graphic Control Label */
      _plotter->write_byte (R___(_plotter) (unsigned char)0xf9);

      /* Block Size (fixed at 4) */
      _plotter->write_byte (R___(_plotter) (unsigned char)4);

      /* Packed fields: Reserved (3 bits), Disposal Method (3 bits),
	 User Input Flag (1 bit), Transparency Flag (final 1 bit) */
      packed_byte = 0;
      if (_plotter->i_transparent)
	packed_byte |= 1;
      if (_plotter->i_transparent && _plotter->i_animation)
	packed_byte |= (DISP_RESTORE_TO_BACKGROUND << 2);
      else
	packed_byte |= (DISP_UNSPECIFIED << 2);
      _plotter->write_byte (R___(_plotter) packed_byte);

      /* Delay time in hundredths of a second [the same for all frames]
	 (2-byte unsigned int) */
      _i_write_short_int (R___(_plotter) (unsigned int)(_plotter->i_delay));

      /* Transparent Color Index [the same for all frames] */ 
     _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_transparent_index);

      /* Block Terminator (0-length data block) */
      _plotter->write_byte (R___(_plotter) (unsigned char)0);
    }

  /* Image Descriptor */

  /* Image Separator */
  _plotter->write_byte (R___(_plotter) (unsigned char)',');

  /* Image Left and Top Positions (w/ respect to logical screen;
     2-byte unsigned ints) */
  _i_write_short_int (R___(_plotter) 0);
  _i_write_short_int (R___(_plotter) 0);

  /* Image Width, Height (2-byte unsigned ints) */
  _i_write_short_int (R___(_plotter) (unsigned int)_plotter->i_xn);
  _i_write_short_int (R___(_plotter) (unsigned int)_plotter->i_yn);

  /* does current frame's color table differ from zeroth frame's color
     table (i.e. GIF file's global color table)? */
  write_local_table 
    = _same_colormap (_plotter->i_colormap, _plotter->i_global_colormap,
		      _plotter->i_num_color_indices,
		      _plotter->i_num_global_color_indices) ? false : true;

  /* Packed fields: Local Color Table (1 bit), Interlace Flag (1 bit), Sort
     Flag (1 bit), Reserved (2 bits), Local Color Table Size (3 bits) */
  packed_bits = 0x00;
  if (write_local_table)
    {
      packed_bits |= 0x80;
      packed_bits |= (IMAX(_plotter->i_bit_depth - 1, 0));
    }
  /* interlace? */
  if (_plotter->i_interlace)
    packed_bits |= 0x40;

  /* write one byte of packed bits */
  _plotter->write_byte (R___(_plotter) (unsigned char)packed_bits);

  /* Local Color Table (expanded to next higher power of 2, with min=2) */
  if (write_local_table)
    {
      for (i = 0; i < (1 << IMAX(_plotter->i_bit_depth, 1)); ++i)
	{
	  _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_colormap[i].red);
	  _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_colormap[i].green);
	  _plotter->write_byte (R___(_plotter) (unsigned char)_plotter->i_colormap[i].blue);
	}
    }

  /* Table-Based Image Data */

  /* LZW Minimum Code Size.  (Minimum number of bits required to represent
     the set of actual pixel values, which will be the same as the bit
     depth, since our allocated color indices are contiguous.  However,
     this has a floor of 2, and, also compression codes must start out one
     bit longer than the floored version, "because of some algorithmic
     constraints".  See i_rle.c.) */
  min_code_size = IMAX(_plotter->i_bit_depth, 2);
  _plotter->write_byte (R___(_plotter) (unsigned char)min_code_size);

  /* initialize pixel scanner */
  _i_start_scan (S___(_plotter));

  /* Image Data, consisting of a sequence of sub-blocks of size at most 
     255 bytes each, encoded as LZW with variable-length code
     (actually, we use miGIF [RLE] rather than LZW; see i_rle.c) */
  {
    rle_out *rle;
    int pixel;
      
#ifdef LIBPLOTTER
    rle = _rle_init (_plotter->outfp, _plotter->outstream,
		     _plotter->i_bit_depth);
#else
    rle = _rle_init (_plotter->outfp,
		     _plotter->i_bit_depth);
#endif
    while ((pixel = _i_scan_pixel (S___(_plotter))) != -1)
      _rle_do_pixel (rle, pixel);
    _rle_terminate (rle);
  }

  /* Block Terminator */
  _plotter->write_byte (R___(_plotter) (unsigned char)0);
}

void
#ifdef _HAVE_PROTOS
_i_write_gif_trailer (S___(Plotter *_plotter))
#else
_i_write_gif_trailer (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* Trailer Block */
  _plotter->write_byte (R___(_plotter) (unsigned char)';');
}

/* reset scanner variables (first pixel scanned is (0,0), i.e. upper
   left-hand corner) */
void
#ifdef _HAVE_PROTOS
_i_start_scan (S___(Plotter *_plotter))
#else
_i_start_scan (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  _plotter->i_pixels_scanned = 0;
  _plotter->i_pass = 0;
  _plotter->i_hot.x = 0;
  _plotter->i_hot.y = 0;  
}

/* Return index (in color table) of pixel under the hot spot, and continue
   the scan by moving to the next pixel.  Return -1 when scan is finished. */
int
#ifdef _HAVE_PROTOS
_i_scan_pixel (S___(Plotter *_plotter))
#else
_i_scan_pixel (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->i_pixels_scanned < _plotter->i_num_pixels)
    {
      miCanvas *canvas;
      int x, y;
      miPixel full_pixel;
      int pixel;

      /* use a libxmi macro, defined in xmi.h, to extract the miPixel at
	 the hotspot; extract index field from it */
      canvas = (miCanvas *)_plotter->i_canvas;
      x = _plotter->i_hot.x;
      y = _plotter->i_hot.y;      
      MI_GET_CANVAS_DRAWABLE_PIXEL(canvas, x, y, full_pixel)
      pixel = full_pixel.u.index;

      _plotter->i_hot.x++;
      if (_plotter->i_hot.x == _plotter->i_xn)
	{
	  _plotter->i_hot.x = 0;

	  if (_plotter->i_interlace == false)
	    _plotter->i_hot.y++;
	  else			/* move to next scan line */
	    {
	      switch (_plotter->i_pass)
		{
		case 0:
		  /* every 8th row, starting with row 0 */
		  _plotter->i_hot.y += 8;
		  if (_plotter->i_hot.y >= _plotter->i_yn)
		    {
		      _plotter->i_pass++;
		      _plotter->i_hot.y = 4;
		    }
		  break;
		case 1:
		  /* every 8th row, starting with row 4 */
		  _plotter->i_hot.y += 8;
		  if (_plotter->i_hot.y >= _plotter->i_yn)
		    {
		      _plotter->i_pass++;
		      _plotter->i_hot.y = 2;
		    }
		  break;
		case 2:
		  /* every 4th row, starting with row 2 */
		  _plotter->i_hot.y += 4;
		  if (_plotter->i_hot.y >= _plotter->i_yn)
		    {
		      _plotter->i_pass++;
		      _plotter->i_hot.y = 1;
		    }
		  break;
		case 3:
		  /* every 2nd row, starting with row 1 */
		  _plotter->i_hot.y += 2;
		  break;
		}
	    }
	}

      _plotter->i_pixels_scanned++;
      return pixel;
    }
  else				/* scan is finished */
    return -1;
}

/* write out an unsigned short int, in range 0..65535, as 2 bytes in
   little-endian order */
void
#ifdef _HAVE_PROTOS
_i_write_short_int (R___(Plotter *_plotter) unsigned int i)
#else
_i_write_short_int (R___(_plotter) i)
     S___(Plotter *_plotter;)
     unsigned int i;
#endif
{
  unsigned char bytes[2];
  
  bytes[0] = (unsigned char)(i & 0xff);
  bytes[1] = (unsigned char)((i >> 8) & 0xff);

  _plotter->write_bytes (R___(_plotter) 2, bytes);
}

/* tear down image, i.e. deallocate libxmi canvas and reset colormap */
void
#ifdef _HAVE_PROTOS
_i_delete_image (S___(Plotter *_plotter))
#else
_i_delete_image (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* deallocate libxmi's drawing canvas (and painted set struct too) */
  miDeleteCanvas ((miCanvas *)_plotter->i_canvas);
  _plotter->i_canvas = (voidptr_t)NULL;
  miDeletePaintedSet ((miPaintedSet *)_plotter->i_painted_set);
  _plotter->i_painted_set = (voidptr_t)NULL;

  /* reset colormap */
  _plotter->i_num_color_indices = 0;

  /* flag color indices in drawing state as bogus */
  _plotter->drawstate->i_pen_color_status = false;
  _plotter->drawstate->i_fill_color_status = false;
  _plotter->drawstate->i_bg_color_status = false;
}

/* compare two partially filled size-256 colormaps for equality */
static bool
#ifdef _HAVE_PROTOS
_same_colormap (plColor cmap1[256], plColor cmap2[256], int num1, int num2)
#else
_same_colormap (cmap1, cmap2, num1, num2)
     plColor cmap1[256], cmap2[256];
     int num1, num2;
#endif
{
  int i;
  
  if (num1 != num2)
    return false;
  for (i = 0; i < num1; i++)
    if ((cmap1[i].red != cmap2[i].red)
	|| (cmap1[i].green != cmap2[i].green)
	|| (cmap1[i].blue != cmap2[i].blue))
      return false;
  return true;
}
