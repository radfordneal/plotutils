/* This file (i_rle.c) is a module that does run-length encoding on
   a sequence of integers ("pixel values"), and writes the resulting
   encoded sequence to an output stream.  The accompanying header file
   (i_rle.h) defines the external interface.  The encoded sequence
   should be GIF-compatible, even though the compression technique
   is not LZW.

   This module encapsulates the miGIF compression routines, originally
   written by der Mouse and ivo.  Their copyright notice is reproduced
   below. */
   

/*-----------------------------------------------------------------------
 *
 * miGIF Compression - mouse and ivo's GIF-compatible compression
 *
 *          -run length encoding compression routines-
 *
 * Copyright (C) 1998 Hutchison Avenue Software Corporation
 *               http://www.hasc.com
 *               info@hasc.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "AS IS." The Hutchison Avenue 
 * Software Corporation disclaims all warranties, either express or implied, 
 * including but not limited to implied warranties of merchantability and 
 * fitness for a particular purpose, with respect to this code and accompanying
 * documentation. 
 * 
 * The miGIF compression routines do not, strictly speaking, generate files 
 * conforming to the GIF spec, since the image data is not LZW-compressed 
 * (this is the point: in order to avoid transgression of the Unisys patent 
 * on the LZW algorithm.)  However, miGIF generates data streams that any 
 * reasonably sane LZW decompresser will decompress to what we want.
 *
 * miGIF compression uses run length encoding. It compresses horizontal runs 
 * of pixels of the same color. This type of compression gives good results
 * on images with many runs, for example images with lines, text and solid 
 * shapes on a solid-colored background. It gives little or no compression 
 * on images with few runs, for example digital or scanned photos.
 *
 *                               der Mouse
 *                      mouse@rodents.montreal.qc.ca
 *            7D C8 61 52 5D E7 2D 39  4E F1 31 3E E8 B3 27 4B
 *
 *                             ivo@hasc.com
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 *
 */

#include "sys-defines.h"	/* libplot-specific */
#include "extern.h"		/* libplot-specific */
#include "i_rle.h"

/* forward references */
static void _block_out ____P((rle_out *rle, unsigned char c));

static void 
#ifdef _HAVE_PROTOS
_write_block (rle_out *rle)
#else
_write_block (rle)
     rle_out *rle;
#endif
{
  if (rle->ofile)
    {
      fputc (rle->oblen, rle->ofile);
      fwrite ((voidptr_t) &(rle->oblock[0]), 1, rle->oblen, rle->ofile);
    }
#ifdef LIBPLOTTER
  else if (rle->outstream)
    {
      rle->outstream->put ((unsigned char)(rle->oblen));
      rle->outstream->write (&(rle->oblock[0]), rle->oblen);
    }
#endif  

  rle->oblen = 0;
}

static void 
#ifdef _HAVE_PROTOS
_block_out (rle_out *rle, unsigned char c)
#else
_block_out (rle, c)
     rle_out *rle;
     unsigned char c;
#endif
{
  rle->oblock[(rle->oblen)++] = c;
  if (rle->oblen >= 255)
    _write_block (rle);
}

static void 
#ifdef _HAVE_PROTOS
_block_flush (rle_out *rle)
#else
_block_flush (rle)
     rle_out *rle;
#endif
{
  if (rle->oblen > 0)
    _write_block (rle);
}

static void 
#ifdef _HAVE_PROTOS
_output (rle_out *rle, int val)
#else
_output (rle, val)
     rle_out *rle;
     int val;
#endif
{
  rle->obuf |= val << rle->obits;
  rle->obits += rle->out_bits;
  while (rle->obits >= 8)
    { 
      _block_out (rle, (unsigned char)(rle->obuf & 0xff));
      rle->obuf >>= 8;
      rle->obits -= 8;
    }
}

static void 
#ifdef _HAVE_PROTOS
_output_flush (rle_out *rle)
#else
_output_flush (rle)
     rle_out *rle;
#endif
{
  if (rle->obits > 0)
    _block_out (rle, (unsigned char)(rle->obuf));
  _block_flush (rle);
}

static void 
#ifdef _HAVE_PROTOS
_did_clear (rle_out *rle)
#else
_did_clear (rle)
     rle_out *rle;
#endif
{
  rle->out_bits = rle->out_bits_init;
  rle->out_bump = rle->out_bump_init;
  rle->out_clear = rle->out_clear_init;
  rle->out_count = 0;
  rle->rl_table_max = 0;
  rle->just_cleared = true;
}

static void 
#ifdef _HAVE_PROTOS
_output_plain (rle_out *rle, int c)
#else
_output_plain (rle, c)
     rle_out *rle;
     int c;
#endif
{
  rle->just_cleared = false;
  _output (rle, c);
  rle->out_count++;
  if (rle->out_count >= rle->out_bump)
    { 
      rle->out_bits++;
      rle->out_bump += 1 << (rle->out_bits - 1);
    }
  if (rle->out_count >= rle->out_clear)
    { 
      _output (rle, rle->code_clear);
      _did_clear (rle);
    }
}

static unsigned int 
#ifdef _HAVE_PROTOS
_isqrt (unsigned int x)
#else
_isqrt (x)
     unsigned int x;
#endif
{
  unsigned int r;
  unsigned int v;

  if (x < 2)
    return x;
  for (v=x, r=1; v; v>>=2, r<<=1)
    ;
  for ( ; ; )
    { 
      v = ((x / r) + r) / 2;
      if ((v == r) || (v == r+1))
	return r;
      r = v;
    }
}

static unsigned int 
#ifdef _HAVE_PROTOS
_compute_triangle_count (unsigned int count, unsigned int nrepcodes)
#else
_compute_triangle_count (count, nrepcodes)
     unsigned int count, nrepcodes;
#endif
{
  unsigned int perrep, cost;

  cost = 0;
  perrep = (nrepcodes * (nrepcodes+1)) / 2;
  while (count >= perrep)
    { 
      cost += nrepcodes;
      count -= perrep;
    }
  if (count > 0)
    { 
      unsigned int n;

      n = _isqrt (count);
      while ((n*(n+1)) >= 2*count)
	n--;
      while ((n*(n+1)) < 2*count)
	n++;
      cost += n;
    }
  return cost;
}

static void 
#ifdef _HAVE_PROTOS
_max_out_clear (rle_out *rle)
#else
_max_out_clear (rle)
     rle_out *rle;
#endif
{
  rle->out_clear = rle->max_ocodes;
}

static void 
#ifdef _HAVE_PROTOS
_reset_out_clear (rle_out *rle)
#else
_reset_out_clear (rle)
     rle_out *rle;
#endif
{
  rle->out_clear = rle->out_clear_init;
  if (rle->out_count >= rle->out_clear)
    { 
      _output (rle, rle->code_clear);
      _did_clear (rle);
    }
}

static void 
#ifdef _HAVE_PROTOS
_rl_flush_fromclear (rle_out *rle, int count)
#else
     _rl_flush_fromclear (rle, count)
     rle_out *rle;
     int count;
#endif
{
  int n;

  _max_out_clear (rle);
  rle->rl_table_pixel = rle->rl_pixel;
  n = 1;
  while (count > 0)
    { 
      if (n == 1)
	{ 
	  rle->rl_table_max = 1;
	  _output_plain (rle, rle->rl_pixel);
	  count--;
	}
      else if (count >= n)
	{ 
	  rle->rl_table_max = n;
	  _output_plain (rle, rle->rl_basecode + n - 2);
	  count -= n;
	}
      else if (count == 1)
	{ 
	  (rle->rl_table_max)++;
	  _output_plain (rle, rle->rl_pixel);
	  count = 0;
	}
      else
	{ 
	  rle->rl_table_max++;
	  _output_plain (rle, rle->rl_basecode+count-2);
	  count = 0;
	}
      if (rle->out_count == 0)
	n = 1;
      else 
	n++;
    }
  _reset_out_clear (rle);
}

static void 
#ifdef _HAVE_PROTOS
_rl_flush_clearorrep (rle_out *rle, int count)
#else
_rl_flush_clearorrep (rle, count)
     rle_out *rle;
     int count;
#endif
{
  int withclr;

  withclr = 1 + _compute_triangle_count ((unsigned int)count, 
					 (unsigned int)(rle->max_ocodes));
  if (withclr < count)
    { 
      _output (rle, rle->code_clear);
      _did_clear (rle);
      _rl_flush_fromclear (rle, count);
    }
  else
    for ( ; count>0; count--)
      _output_plain (rle, rle->rl_pixel);
}

static void 
#ifdef _HAVE_PROTOS
_rl_flush_withtable (rle_out *rle, int count)
#else
_rl_flush_withtable (rle, count)
     rle_out *rle;
     int count;
#endif
{
  int repmax;
  int repleft;
  int leftover;

  repmax = count / rle->rl_table_max;
  leftover = count % rle->rl_table_max;
  repleft = (leftover ? 1 : 0);
  if (rle->out_count + repmax + repleft > rle->max_ocodes)
    { 
      repmax = rle->max_ocodes - rle->out_count;
      leftover = count - (repmax * rle->rl_table_max);
      repleft = 1 + _compute_triangle_count ((unsigned int)leftover,
					     (unsigned int)(rle->max_ocodes));
    }
  if (1 + _compute_triangle_count ((unsigned int)count, 
				   (unsigned int)(rle->max_ocodes)) 
      < repmax + repleft)
    { 
      _output (rle, rle->code_clear);
      _did_clear (rle);
      _rl_flush_fromclear (rle, count);
      return;
    }
  _max_out_clear (rle);
  for ( ; repmax>0; repmax--)
    _output_plain (rle, rle->rl_basecode + rle->rl_table_max - 2);
  if (leftover)
    { 
      if (rle->just_cleared)
	_rl_flush_fromclear (rle, leftover);
      else if (leftover == 1)
	_output_plain (rle, rle->rl_pixel);
      else
	_output_plain (rle, rle->rl_basecode + leftover - 2);
    }
  _reset_out_clear (rle);
}

/* end a run in progress */
static void 
#ifdef _HAVE_PROTOS
_rl_flush (rle_out *rle)
#else
_rl_flush (rle)
     rle_out *rle;
#endif
{
  if (rle->rl_count == 1)	/* not a real run, just output pixel */
    _output_plain (rle, rle->rl_pixel);
  else
    {
      if (rle->just_cleared)
	_rl_flush_fromclear (rle, rle->rl_count);
      else if ((rle->rl_table_max < 2) 
	       || (rle->rl_table_pixel != rle->rl_pixel))
	_rl_flush_clearorrep (rle, rle->rl_count);
      else
	_rl_flush_withtable (rle, rle->rl_count);
    }

  rle->rl_count = 0;
}

/***********************************************************************/
/* 			EXTERNAL INTERFACE			       */
/***********************************************************************/

/* create new RLE struct, which writes to a specified stream */
rle_out *
#ifdef LIBPLOTTER
_rle_init (FILE *fp, ostream *out, int bit_depth)
#else
#ifdef _HAVE_PROTOS
_rle_init (FILE *fp, int bit_depth)
#else
_rle_init (fp, bit_depth)
     FILE *fp;
     int bit_depth;
#endif
#endif
{
  int init_bits;
  rle_out *rle;

  /* Initial length for compression codes, one bit longer than the minimum
     number of bits needed to represent the set of pixel values.  The
     IMAX() and the addition of 1 bit are "because of some algorithmic
     constraints". */
  init_bits = IMAX(bit_depth, 2) + 1;

  rle = (rle_out *)_plot_xmalloc(sizeof(rle_out));
  rle->ofile = fp;
#ifdef LIBPLOTTER
  rle->outstream = out;
#endif
  rle->obuf = 0;
  rle->obits = 0;
  rle->oblen = 0;
  rle->code_clear = 1 << (init_bits - 1); 	   /* 100..000 */
  rle->code_eof = rle->code_clear + 1;    	   /* 100..001 */
  rle->rl_basecode = rle->code_eof + 1;   	   /* 100..010 */
  rle->out_bump_init = (1 << (init_bits - 1)) - 1; /* 011..111 */
  /* for images with a lot of runs, making out_clear_init larger will
     give better compression. */ 
						   /* 011..110 */
  rle->out_clear_init = (init_bits <= 3) ? 9 : (rle->out_bump_init - 1);
  rle->out_bits_init = init_bits;
  rle->max_ocodes = (1 << GIFBITS) - ((1 << (rle->out_bits_init - 1)) + 3);

  _did_clear (rle);
  _output (rle, rle->code_clear);
  rle->rl_count = 0;

  return rle;
}

/* send one pixel to the RLE */
void
#ifdef _HAVE_PROTOS
_rle_do_pixel (rle_out *rle, int c)
#else
_rle_do_pixel (rle, c)
     rle_out *rle;
     int c;
#endif
{
  /* if a run needs to be terminated by being written out, do so */
  if ((rle->rl_count > 0) && (c != rle->rl_pixel))
    _rl_flush (rle);
  /* if current run can be continued, do so (internally) */
  if (rle->rl_pixel == c)
    rle->rl_count++;
  /* otherwise start a new one */
  else
    { 
      rle->rl_pixel = c;
      rle->rl_count = 1;
    }
}

/* flush out any data remaining in RLE; write EOF and deallocate RLE */
void
#ifdef _HAVE_PROTOS
_rle_terminate (rle_out *rle)
#else
_rle_terminate (rle)
     rle_out *rle;
#endif
{
  /* if a run in progress, end it */
  if (rle->rl_count > 0)
    _rl_flush (rle);
  _output (rle, rle->code_eof);
  _output_flush (rle);
  /* deallocate */
  free (rle);
}
