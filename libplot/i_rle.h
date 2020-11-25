/* This header file (i_rle.h) defines the external interface to the module
   i_rle.c, which does run-length encoding on a sequence of integers
   ("pixel values"), and writes the resulting encoded sequence to an output
   stream.  The encoded sequence should be GIF-compatible, even though the
   compression technique is not LZW.

   The module encapsulates the miGIF compression routines, originally
   written by der Mouse and ivo.  Their copyright notice appears in
   i_rle.c. */

/* an `int' should be able to hold 2**GIFBITS distinct values, together
   with -1 */
#define GIFBITS 12

/* the RLE output structure */
typedef struct
{
  int rl_pixel;
  int rl_basecode;
  int rl_count;
  int rl_table_pixel;
  int rl_table_max;
  bool just_cleared;
  int out_bits;
  int out_bits_init;
  int out_count;
  int out_bump;
  int out_bump_init;
  int out_clear;
  int out_clear_init;
  int max_ocodes;
  int code_clear;
  int code_eof;
  unsigned int obuf;
  int obits;
  FILE *ofile;
#ifdef LIBPLOTTER
  ostream *outstream;
#endif
  unsigned char oblock[256];
  int oblen;
} rle_out;

/* create, initialize, and return a new RLE output structure */
#ifdef LIBPLOTTER
extern rle_out *_rle_init ____P((FILE *fp, ostream *out, int bit_depth));
#else
extern rle_out *_rle_init ____P((FILE *fp, int bit_depth));
#endif
/* write a single integer (pixel) to the structure */
extern void _rle_do_pixel ____P((rle_out *rle, int c));
/* wind things up and deallocate the RLE output structure */
extern void _rle_terminate ____P((rle_out *rle));
