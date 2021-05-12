/* FUNCTIONS FOR EXACT SUMMATION. */

/* Copyright 2015, 2018, 2021 Radford M. Neal

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <stdio.h>
#include <string.h>
#include "xsum.h"


/* --------------------------- CONFIGURATION ------------------------------- */


/* IMPLEMENTATION OPTIONS.  Can be set to either 0 or 1, whichever seems
   to be fastest. */

#define USE_MEMSET_SMALL 0  /* Use memset rather than a loop (for small mem)? */
#define USE_MEMSET_LARGE 1  /* Use memset rather than a loop (for large mem)? */
#define USE_USED_LARGE 1    /* Use the used flags in a large accumulator? */

#define OPT_CARRY 1         /* Use manually optimized carry propagation?      */

#define OPT_SMALL_SUM 1     /* Should manually optimized routines be used for */
#define OPT_SMALL_SQNORM 1  /*   operations using the small accumulator?      */
#define OPT_SMALL_DOT 1

#define OPT_LARGE_SUM 1     /* Should manually optimized routines be used for */
#define OPT_LARGE_SQNORM 1  /*   operations using the large accumulator?      */
#define OPT_LARGE_DOT 1

#define OPT_SIMPLE_SUM 1    /* Should manually optimized routines be used for */
#define OPT_SIMPLE_SQNORM 1 /*   operations done with simple FP arithmetic?   */
#define OPT_SIMPLE_DOT 1

#define INLINE_SMALL 1      /* Inline more of the small accumulator routines? */
#define INLINE_LARGE 1      /* Inline more of the large accumulator routines? */


/* UNION OF FLOATING AND INTEGER TYPES. */

union fpunion { xsum_flt fltv; xsum_int intv; xsum_uint uintv; };


/* OPTIONAL INCLUSION OF PBINARY MODULE.  Used for debug output. */

#ifdef PBINARY
# include "pbinary.h"
#else
# define pbinary_int64(x,y) 0
# define pbinary_double(x) 0
#endif


/* SET UP DEBUG FLAG.  It's a variable if debuging is enabled, and a
   constant if disabled (so that no code will be generated then). */

int xsum_debug = 0;

#ifndef DEBUG
# define xsum_debug 0
#endif


/* SET UP INLINE / NOINLINE MACROS. */

#if __GNUC__
# define INLINE inline __attribute__ ((always_inline))
# define NOINLINE __attribute__ ((noinline))
#else
# define INLINE inline 
# define NOINLINE
#endif


/* ------------------------ INTERNAL ROUTINES ------------------------------- */


/* ADD AN INF OR NAN TO A SMALL ACCUMULATOR.  This only changes the flags, 
   not the chunks in the accumulator, which retains the sum of the finite 
   terms (which is perhaps sometimes useful to access, though no function 
   to do so is defined at present).  A NaN with larger payload (seen as a 
   52-bit unsigned integer) takes precedence, with the sign of the NaN always
   being positive.  This ensures that the order of summing NaN values doesn't
   matter. */

static NOINLINE void xsum_small_add_inf_nan 
                       (xsum_small_accumulator *restrict sacc, xsum_int ivalue)
{ 
  xsum_int mantissa;
  union fpunion u;
 
  mantissa = ivalue & XSUM_MANTISSA_MASK;  

  if (mantissa == 0) /* Inf */
  { if (sacc->Inf == 0)   
    { /* no previous Inf */
      sacc->Inf = ivalue;
    }
    else if (sacc->Inf != ivalue)
    { /* previous Inf was opposite sign */
      u.intv = ivalue;
      u.fltv = u.fltv - u.fltv;  /* result will be a NaN */
      sacc->Inf = u.intv;
    }
  }
  else /* NaN */
  { /* Choose the NaN with the bigger payload and clear its sign.  Using <=
       ensures that we will choose the first NaN over the previous zero. */
    if ((sacc->NaN & XSUM_MANTISSA_MASK) <= mantissa)
    { sacc->NaN = ivalue & ~XSUM_SIGN_MASK;
    }
  }
}


/* PROPAGATE CARRIES TO NEXT CHUNK IN A SMALL ACCUMULATOR.  Needs to
   be called often enough that accumulated carries don't overflow out 
   the top, as indicated by sacc->adds_until_propagate.  Returns the 
   index of the uppermost non-zero chunk (0 if number is zero).  

   After carry propagation, the uppermost non-zero chunk will indicate
   the sign of the number, and will not be -1 (all 1s).  It will be in
   the range -2^XSUM_LOW_MANTISSA_BITS to 2^XSUM_LOW_MANTISSA_BITS - 1. 
   Lower chunks will be non-negative, and in the range from 0 up to 
   2^XSUM_LOW_MANTISSA_BITS - 1. */

static NOINLINE int xsum_carry_propagate (xsum_small_accumulator *restrict sacc)
{
  xsum_schunk c, clow, chigh;
  int i, u, uix;

  if (xsum_debug) printf("Carry propagating in small accumulator\n");

  /* Set u to the index of the uppermost non-zero (for now) chunk, or 
     return with value 0 if there is none. */

# if OPT_CARRY
  { u = XSUM_SCHUNKS-1;
    switch (XSUM_SCHUNKS & 0x3) 
    { case 3: if (sacc->chunk[u] != 0) goto found;
              u -= 1;
      case 2: if (sacc->chunk[u] != 0) goto found;
              u -= 1;
      case 1: if (sacc->chunk[u] != 0) goto found;
              u -= 1;
      case 0: ;
    }
    do
    { if (sacc->chunk[u-3] | sacc->chunk[u-2] 
                           | sacc->chunk[u-1] 
                           | sacc->chunk[u]) goto found;
      u -= 4;
    } while (u >= 0);
  
    if (xsum_debug) printf ("number is zero (1)\n");
    uix = 0;
    goto done;
  
  found:
    while (sacc->chunk[u] == 0)
    { u -= 1;
    }
  }
# else
  { for (u = XSUM_SCHUNKS-1; sacc->chunk[u] == 0; u--)
    { if (u == 0) 
      { if (xsum_debug) printf ("number is zero (1)\n");
        uix = 0;
        goto done;
      }
    }
  }
# endif

  if (xsum_debug) printf("u: %d",u);

  /* Carry propagate, starting at the low-order chunks.  Note that the 
     loop limit of u may be increased inside the loop. */

  uix = -1;  /* indicates that a non-zero chunk has not been found yet */

  i = 0;

# if OPT_CARRY
  { 
    /* Quickly skip over unused low-order chunks.  Done here at the start
       on the theory that there are often many unused low-order chunks,
       justifying some overhead to begin, but later stretches of unused
       chunks may not be as large. */

    int e = u-3;
    while (i <= e)
    { if (sacc->chunk[i] | sacc->chunk[i+1]
                         | sacc->chunk[i+2]
                         | sacc->chunk[i+3]) break;
      i += 4;
    }
  }
# endif

  do
  { 
    /* Find the next non-zero chunk, or break out of loop if there is none. */

#   if OPT_CARRY
    { do
      { c = sacc->chunk[i]; 
        if (c != 0) goto nonzero;
        i += 1;
        if (i > u) break;
        c = sacc->chunk[i]; 
        if (c != 0) goto nonzero;
        i += 1;
      } while (i <= u);
    }
#   else
    { do
      { c = sacc->chunk[i]; 
        if (c != 0) goto nonzero;
        i += 1;
      } while (i <= u);
    }
#   endif

    break;

    /* Propagate possible carry from this chunk to next chunk up. */

  nonzero:
    chigh = c >> XSUM_LOW_MANTISSA_BITS;
    if (chigh == 0)
    { uix = i;
      i += 1;
      continue;  /* no need to change this chunk */
    }

    if (u == i)
    { if (chigh == -1)
      { uix = i;
        break;   /* don't propagate -1 into the region of all zeros above */
      }
      u = i+1;   /* we will change chunk[u+1], so we'll need to look at it */
    }

    clow = c & XSUM_LOW_MANTISSA_MASK;
    if (clow != 0)
    { uix = i;
    }

    /* We now change chunk[i] and add to chunk[i+1]. Note that i+1 should be
       in range (no bigger than XSUM_CHUNKS-1) if summing memory, since
       the number of chunks is big enough to hold any sum, and we do not 
       store redundant chunks with values 0 or -1 above previously non-zero 
       chunks.  But other add operations might cause overflow, in which 
       case we produce a NaN with all 1s as payload.  (We can't reliably produce
       an Inf of the right sign.) */

    sacc->chunk[i] = clow;
    if (i+1 >= XSUM_SCHUNKS)
    { xsum_small_add_inf_nan (sacc, 
        ((xsum_int)XSUM_EXP_MASK << XSUM_MANTISSA_BITS) | XSUM_MANTISSA_MASK);
      u = i;
    }
    else
    { sacc->chunk[i+1] += chigh;
    }

    i += 1;

  } while (i <= u);

  if (xsum_debug) printf ("  uix: %d  new u: %d\n", uix,u);

  /* Check again for the number being zero, since carry propagation might
     have created zero from something that initially looked non-zero. */

  if (uix < 0)
  { if (xsum_debug) printf ("number is zero (2)\n");
    uix = 0;
    goto done;
  }

  /* While the uppermost chunk is negative, with value -1, combine it with
     the chunk below (if there is one) to produce the same number but with
     one fewer non-zero chunks. */

  while (sacc->chunk[uix] == -1 && uix > 0)
  { /* A shift of a negative number is undefined according to the standard, so
       do a multiply - it's all presumably constant-folded by the compiler. */
    sacc->chunk[uix-1] += ((xsum_schunk) -1) 
                             * (((xsum_schunk) 1) << XSUM_LOW_MANTISSA_BITS); 
    sacc->chunk[uix] = 0;
    uix -= 1;
  }

  /* We can now add one less than the total allowed terms before the
     next carry propagate. */

done:
  sacc->adds_until_propagate = XSUM_SMALL_CARRY_TERMS-1;

  /* Return index of uppermost non-zero chunk. */

  return uix;
}


/* INITIALIZE LARGE ACCUMULATOR CHUNKS.  Sets all counts to -1. */

static void xsum_large_init_chunks (xsum_large_accumulator *restrict lacc)
{ 
# if USE_MEMSET_LARGE
  {
    /* Since in two's complement representation, -1 consists of all 1 bits,
       we can initialize 16-bit values to -1 by initializing their component
       bytes to 0xff. */

    memset (lacc->count, 0xff, XSUM_LCHUNKS * sizeof *lacc->count);
  }
# else
  { xsum_lcount *p;
    int n;
    p = lacc->count;
    n = XSUM_LCHUNKS;
    do { *p++ = -1; n -= 1; } while (n > 0);
  }
# endif

# if USE_USED_LARGE
#   if USE_MEMSET_SMALL
      memset(lacc->chunks_used, 0, XSUM_LCHUNKS/64 * sizeof *lacc->chunks_used);
#   else
    { xsum_lchunk *p;
      int n;
      p = lacc->chunks_used;
      n = XSUM_LCHUNKS/64;
      do { *p++ = 0; n -= 1; } while (n > 0);
    }
#   endif
    lacc->used_used = 0;
# endif
}


/* ADD CHUNK FROM A LARGE ACCUMULATOR TO THE SMALL ACCUMULATOR WITHIN IT. 
   The large accumulator chunk to add is indexed by ix.  This chunk will
   be cleared to zero and its count reset after it has been added to the
   small accumulator (except no add is done for a new chunk being initialized).
   This procedure should not be called for the special chunks correspnding to
   Inf or NaN, whose counts should always remain at -1. */

#if INLINE_LARGE
  INLINE
#endif
static void xsum_add_lchunk_to_small (xsum_large_accumulator *restrict lacc,
                                      xsum_expint ix)
{
  xsum_expint exp, low_exp, high_exp;
  xsum_uint low_chunk, mid_chunk, high_chunk;
  xsum_lchunk chunk;
  
  const xsum_expint count = lacc->count[ix];

  /* Add to the small accumulator only if the count is not -1, which
     indicates a chunk that contains nothing yet. */

  if (count >= 0)
  {
    /* Propagate carries in the small accumulator if necessary. */

    if (lacc->sacc.adds_until_propagate == 0)
    { (void) xsum_carry_propagate(&lacc->sacc);
    }

    /* Get the chunk we will add.  Note that this chunk is the integer sum
       of entire 64-bit floating-point representations, with sign, exponent, 
       and mantissa, but we want only the sum of the mantissas. */

    chunk = lacc->chunk[ix];

    if (xsum_debug)
    { printf("Adding chunk %d to small accumulator (count %d, chunk %016llx)\n",
              (int) ix, (int) count, (long long) chunk);
    }

    /* If we added the maximum number of values to 'chunk', the sum of
       the sign and exponent parts (all the same, equal to the index) will 
       have overflowed out the top, leaving only the sum of the mantissas.
       If the count of how many more terms we could have summed is greater
       than zero, we therefore add this count times the index (shifted to
       the position of the sign and exponent) to get the unwanted bits to 
       overflow out the top. */

    if (count > 0)
    { chunk += (xsum_lchunk)(count*ix) << XSUM_MANTISSA_BITS;
    }

    /* Find the exponent for this chunk from the low bits of the index, 
       and split it into low and high parts, for accessing the small 
       accumulator.  Noting that for denormalized numbers where the
       exponent part is zero, the actual exponent is 1 (before subtracting
       the bias), not zero. */

    exp = ix & XSUM_EXP_MASK;
    if (exp == 0)
    { low_exp = 1;
      high_exp = 0;
    }
    else
    { low_exp = exp & XSUM_LOW_EXP_MASK;
      high_exp = exp >> XSUM_LOW_EXP_BITS;
    }

    /* Split the mantissa into three parts, for three consecutive chunks in
       the small accumulator.  Except for denormalized numbers, add in the sum
       of all the implicit 1 bits that are above the actual mantissa bits. */

    low_chunk = (chunk << low_exp) & XSUM_LOW_MANTISSA_MASK;
    mid_chunk = chunk >> (XSUM_LOW_MANTISSA_BITS - low_exp);
    if (exp != 0) /* normalized */
    { mid_chunk += (xsum_lchunk)((1 << XSUM_LCOUNT_BITS) - count)
         << (XSUM_MANTISSA_BITS - XSUM_LOW_MANTISSA_BITS + low_exp);
    }
    high_chunk = mid_chunk >> XSUM_LOW_MANTISSA_BITS;
    mid_chunk &= XSUM_LOW_MANTISSA_MASK;

    if (xsum_debug)
    { printf("chunk div: low "); pbinary_int64(low_chunk,64); printf("\n");
      printf("           mid "); pbinary_int64(mid_chunk,64); printf("\n");
      printf("          high "); pbinary_int64(high_chunk,64); printf("\n");
    }

    /* Add or subtract the three parts of the mantissa from three small
       accumulator chunks, according to the sign that is part of the index. */

    if (xsum_debug)
    { printf("Small chunks %d, %d, %d before add or subtract:\n",
              (int)high_exp, (int)high_exp+1, (int)high_exp+2);
      pbinary_int64 (lacc->sacc.chunk[high_exp], 64); printf("\n");
      pbinary_int64 (lacc->sacc.chunk[high_exp+1], 64); printf("\n");
      pbinary_int64 (lacc->sacc.chunk[high_exp+2], 64); printf("\n");
    }

    if (ix & (1 << XSUM_EXP_BITS))
    { lacc->sacc.chunk[high_exp] -= low_chunk;
      lacc->sacc.chunk[high_exp+1] -= mid_chunk;
      lacc->sacc.chunk[high_exp+2] -= high_chunk;
    }
    else
    { lacc->sacc.chunk[high_exp] += low_chunk;
      lacc->sacc.chunk[high_exp+1] += mid_chunk;
      lacc->sacc.chunk[high_exp+2] += high_chunk;
    }

    if (xsum_debug)
    { printf("Small chunks %d, %d, %d after add or subtract:\n",
              (int)high_exp, (int)high_exp+1, (int)high_exp+2);
      pbinary_int64 (lacc->sacc.chunk[high_exp], 64); printf("\n");
      pbinary_int64 (lacc->sacc.chunk[high_exp+1], 64); printf("\n");
      pbinary_int64 (lacc->sacc.chunk[high_exp+2], 64); printf("\n");
    }

    /* The above additions/subtractions reduce by one the number we can
       do before we need to do carry propagation again. */
  
    lacc->sacc.adds_until_propagate -= 1;
  }

  /* We now clear the chunk to zero, and set the count to the number
     of adds we can do before the mantissa would overflow.  We also
     set the bit in chunks_used to indicate that this chunk is in use
     (if that is enabled). */

  lacc->chunk[ix] = 0;
  lacc->count[ix] = 1 << XSUM_LCOUNT_BITS;

# if USE_USED_LARGE
    lacc->chunks_used[ix>>6] |= (xsum_used)1 << (ix & 0x3f);
    lacc->used_used |= (xsum_used)1 << (ix>>6);
# endif
}


/* ADD A CHUNK TO THE LARGE ACCUMULATOR OR PROCESS NAN OR INF.  This routine
   is called when the count for a chunk is negative after decrementing, which 
   indicates either inf/nan, or that the chunk has not been initialized, or
   that the chunk needs to be transferred to the small accumulator. */

#if INLINE_LARGE
  INLINE
#endif
static void xsum_large_add_value_inf_nan (xsum_large_accumulator *restrict lacc,
                                          xsum_expint ix, xsum_lchunk uintv)
{
  if ((ix & XSUM_EXP_MASK) == XSUM_EXP_MASK)
  { xsum_small_add_inf_nan (&lacc->sacc, uintv);
  }
  else
  { xsum_add_lchunk_to_small (lacc, ix);
    lacc->count[ix] -= 1;
    lacc->chunk[ix] += uintv;
  }
}


/* TRANSFER ALL CHUNKS IN LARGE ACCUMULATOR TO ITS SMALL ACCUMULATOR. */

static void xsum_large_transfer_to_small (xsum_large_accumulator *restrict lacc)
{
  if (xsum_debug) printf("Transferring chunks in large accumulator\n");

# if USE_USED_LARGE
  { 
    xsum_used *p, *e;
    xsum_used u, uu;
    int ix;

    p = lacc->chunks_used;
    e = p + XSUM_LCHUNKS/64;

    /* Very quickly skip some unused low-order blocks of chunks by looking 
       at the used_used flags. */

    uu = lacc->used_used;
    if ((uu & 0xffffffff) == 0) { uu >>= 32; p += 32; }
    if ((uu & 0xffff) == 0) { uu >>= 16; p += 16; }
    if ((uu & 0xff) == 0) p += 8;

    /* Loop over remaining blocks of chunks. */

    do 
    { 
      /* Loop to quickly find the next non-zero block of used flags, or finish
         up if we've added all the used blocks to the small accumulator. */

      for (;;)
      { u = *p;
        if (u != 0) break;
        p += 1;
        if (p == e) return;
        u = *p;
        if (u != 0) break;
        p += 1;
        if (p == e) return;
        u = *p;
        if (u != 0) break;
        p += 1;
        if (p == e) return;
        u = *p;
        if (u != 0) break;
        p += 1;
        if (p == e) return;
      }

      /* Find and process the chunks in this block that are used.  We skip
         forward based on the chunks_used flags until we're within eight
         bits of a chunk that is in use. */

      ix = (p - lacc->chunks_used) << 6;
      if ((u & 0xffffffff) == 0) { u >>= 32; ix += 32; }
      if ((u & 0xffff) == 0) { u >>= 16; ix += 16; }
      if ((u & 0xff) == 0) { u >>= 8; ix += 8; }

      do
      { if (lacc->count[ix] >= 0)
        { xsum_add_lchunk_to_small (lacc, ix); 
        }
        ix += 1;
        u >>= 1;
      } while (u != 0);

      p += 1;

    } while (p != e);
  }
# else
  { xsum_expint ix;

    /* When there are no used flags, we scan sequentially for chunks that
       need to be added to the small accumulator. */

    for (ix = 0; ix < XSUM_LCHUNKS; ix++)
    { if (lacc->count[ix] >= 0)
      { xsum_add_lchunk_to_small (lacc, ix);
      }
    }
  }
# endif
}


/* ------------------------ EXTERNAL ROUTINES ------------------------------- */


/* INITIALIZE A SMALL ACCUMULATOR TO ZERO. */

void xsum_small_init (xsum_small_accumulator *restrict sacc)
{ 
# if USE_MEMSET_SMALL
    memset (sacc, 0, sizeof *sacc);
# else
  { xsum_schunk *p;
    int n;
    p = sacc->chunk;
    n = XSUM_SCHUNKS;
    do { *p++ = 0; n -= 1; } while (n > 0);
    sacc->Inf = sacc->NaN = 0;
  } 
# endif
  sacc->adds_until_propagate = XSUM_SMALL_CARRY_TERMS;
}


/* ADD ONE NUMBER TO A SMALL ACCUMULATOR ASSUMING NO CARRY PROPAGATION REQ'D. 
   This function is declared INLINE regardless of the setting of INLINE_SMALL
   and for good performance it must be inlined by the compiler (otherwise the 
   procedure call overhead will result in substantial inefficiency). */

static INLINE void xsum_add1_no_carry (xsum_small_accumulator *restrict sacc, 
                                       xsum_flt value)
{ 
  union fpunion u;
  xsum_int ivalue;
  xsum_int mantissa, low_mantissa, high_mantissa;
  xsum_expint exp, low_exp, high_exp;
  xsum_schunk chunk0, chunk1;
  xsum_schunk *chunk_ptr;

  if (xsum_debug)
  { printf ("ADD1 %+.17le\n     ", (double) value);
    pbinary_double ((double) value);
    printf("\n");
  }

  /* Extract exponent and mantissa. */

  u.fltv = value;
  ivalue = u.intv;

  mantissa = ivalue & XSUM_MANTISSA_MASK;
  exp = (ivalue >> XSUM_MANTISSA_BITS) & XSUM_EXP_MASK;

  /* Categorize number as normal, denormalized, or Inf/NaN according to 
     the value of the exponent field. */

  if (exp == 0) /* zero or denormalized */
  { /* If it's a zero (positive or negative), we do nothing. */
    if (mantissa == 0) 
    { return;
    }
    /* Denormalized mantissa has no implicit 1, but exponent is 1 not 0. */
    exp = 1;
  }
  else if (exp == XSUM_EXP_MASK)  /* Inf or NaN */
  { /* Just update flags in accumulator structure. */
    xsum_small_add_inf_nan (sacc, ivalue);
    return;
  }
  else /* normalized */
  { /* OR in implicit 1 bit at top of mantissa */
    mantissa |= (xsum_int)1 << XSUM_MANTISSA_BITS;
  }

  /* Separate high part of exponent, used as index of chunk, and low
     part of exponent, giving position within chunk. */

  low_exp = exp & XSUM_LOW_EXP_MASK;
  high_exp = exp >> XSUM_LOW_EXP_BITS;

  if (xsum_debug)
  { printf("  high exp: ");
    pbinary_int64 (high_exp, XSUM_HIGH_EXP_BITS);
    printf("  low exp: ");
    pbinary_int64 (low_exp, XSUM_LOW_EXP_BITS);
    printf("\n");
  }

  chunk_ptr = sacc->chunk + high_exp;
  chunk0 = chunk_ptr[0];
  chunk1 = chunk_ptr[1];

  /* Separate mantissa into two parts, after shifting, and add to (or 
     subtract from) this chunk and the next higher chunk (which always 
     exists since there are three extra ones at the top). 

     Note that low_mantissa will have at most XSUM_LOW_MANTISSA_BITS bits,
     while high_mantissa will have at most XSUM_MANTISSA_BITS bits, since
     even though high_mantissa includes the extra implicit 1 bit, it will
     also be shifted right by at least one bit. */

  low_mantissa = ((xsum_uint)mantissa << low_exp) & XSUM_LOW_MANTISSA_MASK;
  high_mantissa = mantissa >> (XSUM_LOW_MANTISSA_BITS - low_exp);

  /* Add or subtract to or from the two affected chunks. */

  if (ivalue < 0)
  { 
    chunk_ptr[0] = chunk0 - low_mantissa;
    chunk_ptr[1] = chunk1 - high_mantissa;

    if (xsum_debug)
    { printf (" -high man: ");
      pbinary_int64 (-high_mantissa, XSUM_MANTISSA_BITS);
      printf ("\n  -low man: ");
      pbinary_int64 (-low_mantissa, XSUM_LOW_MANTISSA_BITS);
      printf("\n");
    }
  }
  else
  { 
    chunk_ptr[0] = chunk0 + low_mantissa;
    chunk_ptr[1] = chunk1 + high_mantissa;

    if (xsum_debug)
    { printf ("  high man: ");
      pbinary_int64 (high_mantissa, XSUM_MANTISSA_BITS);
      printf ("\n   low man: ");
      pbinary_int64 (low_mantissa, XSUM_LOW_MANTISSA_BITS);
      printf("\n");
    }
  }
}


/* ADD ONE DOUBLE TO A SMALL ACCUMULATOR.  This is equivalent to, but 
   somewhat faster than, calling xsum_small_addv with a vector of one
   value. */

void xsum_small_add1 (xsum_small_accumulator *restrict sacc, xsum_flt value)
{ 
  if (sacc->adds_until_propagate == 0)
  { (void) xsum_carry_propagate(sacc);
  }

  xsum_add1_no_carry (sacc, value);

  sacc->adds_until_propagate -= 1;
}


/* ADD A VECTOR TO A SMALL ACCUMULATOR, ASSUMING NO CARRY PROPAGATION NEEDED.
   Adds n-1 numbers from vec, which must have at least n elements; n must
   be at least 1.  This odd specificiation is designed so that in the OPT
   version we can pre-fetch the next value to allow some time for memory 
   response before the value is used. */

#if INLINE_SMALL
  INLINE
#endif
static void xsum_addv_no_carry (xsum_small_accumulator *restrict sacc, 
                                const xsum_flt *restrict vec, 
                                xsum_length n)
{ 
# if OPT_SMALL_SUM
  { xsum_flt f;
    for (;;)
    { f = *vec;
      vec += 1; 
      n -= 1;
      if (n == 0) break;
      xsum_add1_no_carry (sacc, f);
    }
  }
# else
  { xsum_length i;
    n -= 1;
    for (i = 0; i < n; i++)
    { xsum_add1_no_carry (sacc, vec[i]);
    }
  }
# endif
}


/* ADD A VECTOR OF FLOATING-POINT NUMBERS TO A SMALL ACCUMULATOR.  Mixes
   calls of xsum_carry_propagate with calls of xsum_addv_no_carry to add 
   parts that are small enough that no carry will result.  Note that
   xsum_addv_no_carry may pre-fetch one beyond the last value it sums, 
   so to be safe, adding the last value has to be done separately at
   the end. */

void xsum_small_addv (xsum_small_accumulator *restrict sacc, 
                      const xsum_flt *restrict vec, 
                      xsum_length n)
{ xsum_length m;

  if (n == 0) return;

  while (n > 1)
  { if (sacc->adds_until_propagate == 0)
    { (void) xsum_carry_propagate(sacc);
    }
    m = n-1 <= sacc->adds_until_propagate ? n-1 : sacc->adds_until_propagate;
    xsum_addv_no_carry (sacc, vec, m+1);
    sacc->adds_until_propagate -= m;
    vec += m; 
    n -= m;
  }

  xsum_small_add1 (sacc, *vec);
}


/* ADD SQUARED NORM OF VECTOR TO SMALL ACCUMULATOR, ASSUME NO CARRY NEEDED.
   Adds n-1 squares of numbers from vec, which must have at least n elements; 
   n must be at least 1.  This odd specificiation is designed so that in the 
   OPT version we can pre-fetch the next value to allow some time for memory 
   response before the value is used. */

#if INLINE_SMALL
  INLINE
#endif
static void xsum_add_sqnorm_no_carry (xsum_small_accumulator *restrict sacc, 
                                      const xsum_flt *restrict vec, 
                                      xsum_length n)
{ 
# if OPT_SMALL_SQNORM
  { xsum_flt f, g;
    f = *vec;
    for (;;)
    { vec += 1; 
      n -= 1;
      if (n == 0) break;
      g = f*f;
      f = *vec;
      xsum_add1_no_carry (sacc, g);
    }
  }
# else
  { xsum_flt f;
    xsum_length i;
    n -= 1;
    for (i = 0; i < n; i++)
    { f = vec[i];
      xsum_add1_no_carry (sacc, f*f);
    }
  }
# endif
}


/* ADD SQUARED NORM OF VECTOR OF FLOATING-POINT NUMBERS TO SMALL ACCUMULATOR.
   Mixes calls of xsum_carry_propagate with calls of xsum_add_sqnorm_no_carry 
   to add parts that are small enough that no carry will result.  Note that
   xsum_add_sqnorm_no_carry may pre-fetch one beyond the last value it sums, 
   so to be safe, adding the last value has to be done separately at
   the end. */

void xsum_small_add_sqnorm (xsum_small_accumulator *restrict sacc, 
                            const xsum_flt *restrict vec, 
                            xsum_length n)
{ xsum_length m;

  if (n == 0) return;

  while (n > 1)
  { if (sacc->adds_until_propagate == 0)
    { (void) xsum_carry_propagate(sacc);
    }
    m = n-1 <= sacc->adds_until_propagate ? n-1 : sacc->adds_until_propagate;
    xsum_add_sqnorm_no_carry (sacc, vec, m+1);
    sacc->adds_until_propagate -= m;
    vec += m; 
    n -= m;
  }

  xsum_small_add1 (sacc, *vec * *vec);
}


/* ADD DOT PRODUCT OF VECTORS TO SMALL ACCUMULATOR, ASSUME NO CARRY NEEDED.
   Adds n-1 products of numbers from vec1 and vec2, which must have at least
   n elements; n must be at least 1.  This odd specificiation is designed so 
   that in the OPT version we can pre-fetch the next values to allow some time
   for memory response before the value is used. */

#if INLINE_SMALL
  INLINE
#endif
static void xsum_add_dot_no_carry (xsum_small_accumulator *restrict sacc, 
                                   const xsum_flt *vec1, const xsum_flt *vec2, 
                                   xsum_length n)
{ 
# if OPT_SMALL_DOT
  { xsum_flt f1, f2, g;
    f1 = *vec1;
    f2 = *vec2;
    for (;;)
    { vec1 += 1; 
      vec2 += 1; 
      n -= 1;
      if (n == 0) break;
      g = f1*f2;
      f1 = *vec1;
      f2 = *vec2;
      xsum_add1_no_carry (sacc, g);
    }
  }
# else
  { xsum_length i;
    n -= 1;
    for (i = 0; i < n; i++)
    { xsum_add1_no_carry (sacc, vec1[i] * vec2[i]);
    }
  }
# endif
}


/* ADD DOT PRODUCT OF VECTORS FLOATING-POINT NUMBERS TO SMALL ACCUMULATOR.
   Mixes calls of xsum_carry_propagate with calls of xsum_add_dot_no_carry 
   to add parts that are small enough that no carry will result.  Note that
   xsum_add_dot_no_carry may pre-fetch one beyond the last value it sums, 
   so to be safe, adding the last value has to be done separately at
   the end. */

void xsum_small_add_dot (xsum_small_accumulator *restrict sacc, 
                         const xsum_flt *vec1, const xsum_flt *vec2, 
                         xsum_length n)
{ xsum_length m;

  if (n == 0) return;

  while (n > 1)
  { if (sacc->adds_until_propagate == 0)
    { (void) xsum_carry_propagate(sacc);
    }
    m = n-1 <= sacc->adds_until_propagate ? n-1 : sacc->adds_until_propagate;
    xsum_add_dot_no_carry (sacc, vec1, vec2, m+1);
    sacc->adds_until_propagate -= m;
    vec1 += m; 
    vec2 += m; 
    n -= m;
  }

  xsum_small_add1 (sacc, *vec1 * *vec2);
}


/* ADD A SMALL ACCUMULATOR TO ANOTHER SMALL ACCUMULATOR.  The first argument
   is the destination, which is modified.  The second is the accumulator to
   add, which may also be modified, but should still represent the same
   number.  Source and destination must be different. */

void xsum_small_add_accumulator (xsum_small_accumulator *restrict dst_sacc, 
                                 xsum_small_accumulator *restrict src_sacc)
{
  int i;

  if (xsum_debug) printf("Adding accumulator to a small accumulator\n");
  if (dst_sacc == src_sacc) abort();

  xsum_carry_propagate (dst_sacc);
  xsum_carry_propagate (src_sacc);

  if (src_sacc->Inf) xsum_small_add_inf_nan (dst_sacc, src_sacc->Inf);
  if (src_sacc->NaN) xsum_small_add_inf_nan (dst_sacc, src_sacc->NaN);
  
  for (i = 0; i < XSUM_SCHUNKS; i++)
  { dst_sacc->chunk[i] += src_sacc->chunk[i];
  }

  dst_sacc->adds_until_propagate = XSUM_SMALL_CARRY_TERMS-2;
}


/* RETURN THE RESULT OF ROUNDING A SMALL ACCUMULATOR.  The rounding mode 
   is to nearest, with ties to even.  The small accumulator may be modified 
   by this operation (by carry propagation being done), but the value it
   represents should not change. */

xsum_flt xsum_small_round (xsum_small_accumulator *restrict sacc)
{ 
  xsum_int ivalue;
  xsum_schunk lower;
  union fpunion u;
  int i, j, e, more;

  if (xsum_debug) printf("Rounding small accumulator\n");

  /* See if we have a NaN from one of the numbers being a NaN, in which
     case we return the NaN with largest payload. */

  if (sacc->NaN != 0)
  { u.intv = sacc->NaN;
    return u.fltv;
  }

  /* Otherwise, if any number was infinite, we return +Inf, -Inf, or a Nan 
     (if both +Inf and -Inf occurred).  Note that we do NOT return NaN if
     we have both an infinite number and a sum of other numbers that 
     overflows with opposite sign, since there is no real ambiguity in 
     such a case. */

  if (sacc->Inf != 0)
  { u.intv = sacc->Inf;
    return u.fltv;
  }

  /* If none of the numbers summed were infinite or NaN, we proceed to
     propagate carries, as a preliminary to finding the magnitude of
     the sum.  This also ensures that the sign of the result can be
     determined from the uppermost non-zero chunk.  

     We also find the index, i, of this uppermost non-zero chunk, as 
     the value returned by xsum_carry_propagate, and set ivalue to 
     sacc->chunk[i].  Note that ivalue will not be 0 or -1, unless 
     i is 0 (the lowest chunk), in which case it will be handled by
     the code for denormalized numbers. */

  i = xsum_carry_propagate(sacc);

  if (xsum_debug) xsum_small_display(sacc);

  ivalue = sacc->chunk[i];

  /* Handle a possible denormalized number, including zero. */

  if (i <= 1)
  { 
    /* Check for zero value, in which case we can return immediately. */

    if (ivalue == 0)
    { return 0.0;
    }

    /* Check if it is actually a denormalized number.  It always is if only
       the lowest chunk is non-zero.  If the highest non-zero chunk is the
       next-to-lowest, we check the magnitude of the absolute value.  
       Note that the real exponent is 1 (not 0), so we need to shift right
       by 1 here, which also means there will be no overflow from the left 
       shift below (but must view absolute value as unsigned). */

    if (i == 0)
    { u.intv = ivalue >= 0 ? ivalue : -ivalue;
      u.intv >>= 1;
      if (ivalue < 0) 
      { u.intv |= XSUM_SIGN_MASK;
      }
      return u.fltv;
    }
    else
    { /* Note: Left shift of -ve number is undefined, so do a multiply instead,
               which is probably optimized to a shift. */
      u.intv = ivalue * ((xsum_int)1 << (XSUM_LOW_MANTISSA_BITS-1))
                 + (sacc->chunk[0] >> 1);
      if (u.intv < 0)
      { if (u.intv > - ((xsum_int)1 << XSUM_MANTISSA_BITS))
        { u.intv = (-u.intv) | XSUM_SIGN_MASK;
          return u.fltv;
        }
      }
      else
      { if (u.uintv < (xsum_uint)1 << XSUM_MANTISSA_BITS)
        { return u.fltv;
        }
      }
      /* otherwise, it's not actually denormalized, so fall through to below */
    }
  }

  /* Find the location of the uppermost 1 bit in the absolute value of the
     upper chunk by converting it (as a signed integer) to a floating point 
     value, and looking at the exponent.  Then set 'more' to the number of
     bits from the lower chunk (and maybe the next lower) that are needed 
     to fill out the mantissa of the result, plus an extra bit to help decide 
     on rounding.  For negative numbers, it may turn out later that we need 
     another bit because negating a negative value may carry out of the top 
     here, but not once more bits are shifted into the bottom later on. */

  u.fltv = (xsum_flt) ivalue;
  e = (u.uintv >> XSUM_MANTISSA_BITS) & XSUM_EXP_MASK;
  more = 1 + XSUM_MANTISSA_BITS + XSUM_EXP_BIAS - e;

  if (xsum_debug)
  { printf("e: %d, more: %d, ivalue: %016llx\n",e,more,(long long)ivalue);
  }

  /* Change 'ivalue' to put in 'more' bits from lower chunks into the bottom.
     Also set 'j' to the index of the lowest chunk from which these bits came,
     and 'lower' to the remaining bits of that chunk not now in 'ivalue'.
     We make sure that 'lower' initially has at least one bit in it, which
     we can later move into 'ivalue' if it turns out that one more bit is 
     needed. */

  ivalue *= (xsum_int)1 << more;  /* multiply, since << of negative undefined */
  if (xsum_debug) 
  { printf("after ivalue <<= more, ivalue: %016llx\n",(long long)ivalue);
  }
  j = i-1;
  lower = sacc->chunk[j];  /* must exist, since denormalized if i==0 */
  if (more >= XSUM_LOW_MANTISSA_BITS)
  { more -= XSUM_LOW_MANTISSA_BITS;
    ivalue += lower << more;
    if (xsum_debug) 
    { printf("after ivalue += lower << more, ivalue: %016llx\n",
              (long long)ivalue);
    }
    j -= 1;
    lower = j < 0 ? 0 : sacc->chunk[j];
  }
  ivalue += lower >> (XSUM_LOW_MANTISSA_BITS - more);
  lower &= ((xsum_schunk)1 << (XSUM_LOW_MANTISSA_BITS - more)) - 1;

  if (xsum_debug)
  { printf("j: %d, new e: %d, new |ivalue|: %016llx, lower: %016llx\n",
            j, e, (long long) (ivalue<0 ? -ivalue : ivalue), (long long)lower);
    
  }

  /* Check for a negative 'ivalue' that when negated doesn't contain a full
     mantissa's worth of bits, plus one to help rounding.  If so, move one
     more bit into 'ivalue' from 'lower' (and remove it from 'lower'). 
     Note that more than one additional bit will not be required because
     xsum_carry_propagate ensures the uppermost non-zero chunk is not -1. */

  if (ivalue < 0 && ((-ivalue) & ((xsum_int)1 << (XSUM_MANTISSA_BITS+1))) == 0)
  { int pos = (xsum_schunk)1 << (XSUM_LOW_MANTISSA_BITS - 1 - more);
    ivalue *= 2;  /* note that left shift undefined if ivalue is negative */
    if (lower & pos)
    { ivalue |= 1;
      lower &= ~pos;
    }
    e -= 1;
  }

  if (xsum_debug)
  { printf("j: %d, new e: %d, new |ivalue|: %016llx, lower: %016llx\n",
            j, e, (long long) (ivalue<0 ? -ivalue : ivalue), (long long)lower);
    
  }

  /* Set u.intv to have just the correct sign bit (rest zeros), and 'ivalue'
     to now have the absolute value of the mantissa. */

  if (ivalue >= 0)
  { u.intv = 0;
  }
  else
  { ivalue = -ivalue;
    u.intv = XSUM_SIGN_MASK;
  }

  if (xsum_debug && (ivalue >> (XSUM_MANTISSA_BITS+1)) != 1) abort();

  /* Round to nearest, with ties to even. At this point, 'ivalue' has the
     absolute value of the number to be rounded, including an extra bit at 
     the bottom.  Bits below that are in 'lower' and in the chunks
     indexed by 'j' and below.  Note that the bits in 'lower' and the chunks 
     below add to the magnitude of the remainder if the number is positive, 
     but subtract from this magnitude if the number is negative. 

     This code goes to done_rounding if it finds that just discarding lower
     order bits is correct, and to round_away_from_zero if instead the 
     magnitude should be increased by one in the lowest bit. */

  if ((ivalue & 1) == 0)  /* extra bit is 0 */
  { if (xsum_debug) 
    { printf("round toward zero, since remainder magnitude is < 1/2\n");
    }
    goto done_rounding;
  }

  if (u.intv == 0)  /* number is positive */
  { if ((ivalue & 2) != 0)  /* low bit 1 (odd) */
    { if (xsum_debug) 
      { printf("round away from zero, since magnitude >= 1/2, goes to even\n");
      }
      goto round_away_from_zero;
    }
    if (lower != 0)
    { if (xsum_debug) 
      { printf("round away from zero, since magnitude > 1/2 (from 'lower')\n");
      }
      goto round_away_from_zero;
    }
  }
  else  /* number is negative */
  { if ((ivalue & 2) == 0)  /* low bit 0 (even) */
    { if (xsum_debug) 
      { printf("round toward zero, since magnitude <= 1/2, goes to even\n");
      }
      goto done_rounding;
    }
    if (lower != 0)
    { if (xsum_debug) 
      { printf("round toward zero, since magnitude < 1/2 (from 'lower')\n");
      }
      goto done_rounding;
    }
  }

  /* If we get here, 'lower' is zero.  We need to look at chunks lower down 
     to see if any are non-zero. */

  while (j > 0)
  { j -= 1;
    if (sacc->chunk[j] != 0)
    { lower = 1;
      break;
    }
  }

  if (u.intv == 0)  /* number is positive, low bit 0 (even) */
  { if (lower != 0)
    { if (xsum_debug) 
      { printf("round away from zero, since magnitude > 1/2 (low chunks)\n");
      }
      goto round_away_from_zero;
    }
    else
    { if (xsum_debug) 
      { printf("round toward zero, magnitude == 1/2 (low chunks)\n");
      }
      goto done_rounding;
    }
  }
  else  /* number is negative, low bit 1 (odd) */
  { if (lower != 0)
    { if (xsum_debug) 
      { printf("round toward zero, since magnitude < 1/2 (low chunks)\n");
      }
      goto done_rounding;
    }
    else
    { if (xsum_debug) 
      { printf("round away from zero, magnitude == 1/2 (low chunks)\n");
      }
      goto round_away_from_zero;
    }
  }

round_away_from_zero:

  /* Round away from zero, then check for carry having propagated out the 
     top, and shift if so. */

  ivalue += 2;
  if (ivalue & ((xsum_int)1 << (XSUM_MANTISSA_BITS+2)))
  { ivalue >>= 1;
    e += 1;
  }

done_rounding: ;

  /* Get rid of the bottom bit that was used to decide on rounding. */

  ivalue >>= 1;

  /* Adjust to the true exponent, accounting for where this chunk is. */

  e += (i<<XSUM_LOW_EXP_BITS) - XSUM_EXP_BIAS - XSUM_MANTISSA_BITS;

  /* If exponent has overflowed, change to plus or minus Inf and return. */

  if (e >= XSUM_EXP_MASK) 
  { u.intv |= (xsum_int) XSUM_EXP_MASK << XSUM_MANTISSA_BITS;
    if (xsum_debug)
    { printf ("Final rounded result: %.17le (overflowed)\n  ", u.fltv);
      pbinary_double(u.fltv);
      printf("\n");
    }
    return u.fltv;
  }

  /* Put exponent and mantissa into u.intv, which already has the sign,
     then return u.fltv. */

  u.intv += (xsum_int)e << XSUM_MANTISSA_BITS; 
  u.intv += ivalue & XSUM_MANTISSA_MASK;  /* mask out the implicit 1 bit */

  if (xsum_debug)
  { if ((ivalue >> XSUM_MANTISSA_BITS) != 1) abort();
    printf ("Final rounded result: %.17le\n  ", u.fltv);
    pbinary_double(u.fltv);
    printf("\n");
  }

  return u.fltv;
}


/* INITIALIZE A LARGE ACCUMULATOR TO ZERO. */

void xsum_large_init (xsum_large_accumulator *restrict lacc)
{ 
  xsum_large_init_chunks (lacc);
  xsum_small_init (&lacc->sacc);
}


/* ADD A VECTOR OF FLOATING-POINT NUMBERS TO A LARGE ACCUMULATOR. */

void xsum_large_addv (xsum_large_accumulator *restrict lacc,
                      const xsum_flt *restrict vec, 
                      xsum_length n)
{
  if (xsum_debug) printf("LARGE ADDV OF %ld VALUES\n",(long)n);

  if (n == 0) return;

# if OPT_LARGE_SUM
  { 
    /* Version that's been manually optimized:  Loop unrolled, pre-fetch
       attempted, branches eliminated, ... */

    union fpunion u1, u2;
    int count1, count2;
    xsum_expint ix1, ix2;
    const xsum_flt *v;
    xsum_length m;
    xsum_flt f;

    v = vec;
    f = *v;

    /* Unrolled loop processing two values each time around.  The loop is
       done as two nested loops, arranged so that the inner one will have
       no branches except for the one looping back.  This is achieved by
       a trick for combining three tests for negativity into one.  The
       last one or two values are not done here, so that the pre-fetching
       will not go past the end of the array (which would probably be OK,
       but is technically not allowed). */

    m = n-3; /* leave out last one or two, terminate when negative, for trick */
    while (m >= 0)
    { 
      /* Loop processing two values at a time until we're done, or until 
         one (or both) of the values result in a chunk needing to be processed. 
         Updates are done here for both of these chunks, even though it is not
         yet known whether these updates ought to have been done.  We hope
         this allows for better memory pre-fetch and instruction scheduling. */

      do
      { v += 1;
        u2.fltv = *v;
        u1.fltv = f;

        ix1 = u1.uintv >> XSUM_MANTISSA_BITS;
        count1 = lacc->count[ix1] - 1;
        lacc->count[ix1] = count1;
        lacc->chunk[ix1] += u1.uintv;

        v += 1;
        f = *v;

        ix2 = u2.uintv >> XSUM_MANTISSA_BITS;
        count2 = lacc->count[ix2] - 1;
        lacc->count[ix2] = count2;
        lacc->chunk[ix2] += u2.uintv;

        m -= 2;

      } while (((xsum_length)count1 | (xsum_length)count2 | m) >= 0);
           /* ... equivalent to while (count1 >= 0 && count2 >= 0 && m >= 0) */

      /* See if we were actually supposed to update these chunks.  If not,
         back out the changes and then process the chunks as they ought to
         have been processed. */

      if (count1 < 0 || count2 < 0)
      { lacc->count[ix2] = count2 + 1;
        lacc->chunk[ix2] -= u2.uintv;
        if (count1 < 0)
        { lacc->count[ix1] = count1 + 1;
          lacc->chunk[ix1] -= u1.uintv;
          xsum_large_add_value_inf_nan (lacc, ix1, u1.uintv);
          count2 = lacc->count[ix2] - 1;
        }
        if (count2 < 0)
        { xsum_large_add_value_inf_nan (lacc, ix2, u2.uintv);
        }
        else
        { lacc->count[ix2] = count2;
          lacc->chunk[ix2] += u2.uintv;
        }
      }

    }

    /* Process the last one or two values, without pre-fetching. */

    m += 3;
    for (;;)
    { u1.fltv = f;
      ix1 = u1.uintv >> XSUM_MANTISSA_BITS;
      count1 = lacc->count[ix1] - 1;
      if (count1 < 0)
      { xsum_large_add_value_inf_nan (lacc, ix1, u1.uintv);
      }
      else
      { lacc->count[ix1] = count1;
        lacc->chunk[ix1] += u1.uintv;
      }
      m -= 1;
      if (m == 0) break;
      v += 1;
      f = *v;
    }
  }
# else
  { 
    /* Version not manually optimized - maybe the compiler can do better. */

    union fpunion u;
    xsum_lcount count;
    xsum_expint ix;

    do
    { 
      /* Fetch the next number, and convert to integer form in u.uintv. */

      u.fltv = *vec;
      vec += 1;

      /* Isolate the upper sign+exponent bits that index the chunk. */

      ix = u.uintv >> XSUM_MANTISSA_BITS;

      /* Find the count for this chunk, and subtract one. */

      count = lacc->count[ix] - 1;

      if (count < 0)
      { 
        /* If the decremented count is negative, it's either a special 
           Inf/NaN chunk (in which case count will stay at -1), or one that 
           needs to be transferred to the small accumulator, or one that
           has never been used before and needs to be initialized. */

        xsum_large_add_value_inf_nan (lacc, ix, u.uintv);
      }
      else
      { 
        /* Store the decremented count of additions allowed before transfer,
           and add this value to the chunk. */

        lacc->count[ix] = count;
        lacc->chunk[ix] += u.uintv;
      }

      n -= 1;

    } while (n > 0);
  }
# endif
}


/* ADD ONE DOUBLE TO A LARGE ACCUMULATOR.  Just calls xsum_large_addv. */

void xsum_large_add1 (xsum_large_accumulator *restrict lacc, xsum_flt value)
{ 
  xsum_large_addv (lacc, &value, 1);
}


/* ADD SQUARED NORM OF VECTOR OF FLOATING-POINT NUMBERS TO LARGE ACCUMULATOR. */

void xsum_large_add_sqnorm (xsum_large_accumulator *restrict lacc,
                            const xsum_flt *restrict vec, 
                            xsum_length n)
{ 
  if (xsum_debug) printf("LARGE ADD_SQNORM OF %ld VALUES\n",(long)n);

  if (n == 0) return;

# if OPT_LARGE_SQNORM
  { 
    union fpunion u1, u2;
    int count1, count2;
    xsum_expint ix1, ix2;
    const xsum_flt *v;
    xsum_flt f;
    xsum_length m;

    v = vec;
    f = *v;

    /* Unrolled loop processing two squares each time around.  The loop is
       done as two nested loops, arranged so that the inner one will have
       no branches except for the one looping back.  This is achieved by
       a trick for combining three tests for negativity into one.  The
       last one or two squares are not done here, so that the pre-fetching
       will not go past the end of the array (which would probably be OK,
       but is technically not allowed). */

    m = n-3; /* leave out last one or two, terminate when negative, for trick */
    while (m >= 0)
    { 
      /* Loop processing two squares at a time until we're done, or until 
         one (or both) of them result in a chunk needing to be processed. 
         Updates are done here for both of these chunks, even though it is not
         yet known whether these updates ought to have been done.  We hope
         this allows for better memory pre-fetch and instruction scheduling. */

      do
      { u1.fltv = f * f;
        v += 1;
        f = *v;
        u2.fltv = f * f;

        ix1 = u1.uintv >> XSUM_MANTISSA_BITS;
        count1 = lacc->count[ix1] - 1;
        lacc->count[ix1] = count1;
        lacc->chunk[ix1] += u1.uintv;

        v += 1;
        f = *v;

        ix2 = u2.uintv >> XSUM_MANTISSA_BITS;
        count2 = lacc->count[ix2] - 1;
        lacc->count[ix2] = count2;
        lacc->chunk[ix2] += u2.uintv;

        m -= 2;

      } while (((xsum_length)count1 | (xsum_length)count2 | m) >= 0);
           /* ... equivalent to while (count1 >= 0 && count2 >= 0 && m >= 0) */

      /* See if we were actually supposed to update these chunks.  If not,
         back out the changes and then process the chunks as they ought to
         have been processed. */

      if (count1 < 0 || count2 < 0)
      { lacc->count[ix2] = count2 + 1;
        lacc->chunk[ix2] -= u2.uintv;
        if (count1 < 0)
        { lacc->count[ix1] = count1 + 1;
          lacc->chunk[ix1] -= u1.uintv;
          xsum_large_add_value_inf_nan (lacc, ix1, u1.uintv);
          count2 = lacc->count[ix2] - 1;
        }
        if (count2 < 0)
        { xsum_large_add_value_inf_nan (lacc, ix2, u2.uintv);
        }
        else
        { lacc->count[ix2] = count2;
          lacc->chunk[ix2] += u2.uintv;
        }
      }

    }

    /* Process the last one or two squares, without pre-fetching. */

    m += 3;
    for (;;)
    { u1.fltv = f * f;
      ix1 = u1.uintv >> XSUM_MANTISSA_BITS;
      count1 = lacc->count[ix1] - 1;
      if (count1 < 0)
      { xsum_large_add_value_inf_nan (lacc, ix1, u1.uintv);
      }
      else
      { lacc->count[ix1] = count1;
        lacc->chunk[ix1] += u1.uintv;
      }
      m -= 1;
      if (m == 0) break;
      v += 1;
      f = *v;
    }
  }
# else
  {
    /* Version not manually optimized - maybe the compiler can do better. */

    union fpunion u;
    xsum_lcount count;
    xsum_expint ix;

    do
    {
      /* Fetch the next number, square it, and convert to integer form in 
         u.uintv. */

      double a = *vec;
      u.fltv = a*a;
      vec += 1;

      /* Isolate the upper sign+exponent bits that index the chunk. */

      ix = u.uintv >> XSUM_MANTISSA_BITS;

      /* Find the count for this chunk, and subtract one. */

      count = lacc->count[ix] - 1;

      if (count < 0)
      {
        /* If the decremented count is negative, it's either a special 
           Inf/NaN chunk (in which case count will stay at -1), or one that 
           needs to be transferred to the small accumulator, or one that
           has never been used before and needs to be initialized. */

        xsum_large_add_value_inf_nan (lacc, ix, u.uintv);
      }
      else
      {
        /* Store the decremented count of additions allowed before transfer,
           and add this value to the chunk. */

        lacc->count[ix] = count;
        lacc->chunk[ix] += u.uintv;
      }
      n -= 1;
    } while (n > 0);
  }
# endif
}


/* ADD DOT PRODUCT OF VECTORS OF FLOATING-POINT NUMBERS TO LARGE ACCUMULATOR. */

void xsum_large_add_dot (xsum_large_accumulator *restrict lacc,
                         const xsum_flt *vec1, 
                         const xsum_flt *vec2, 
                         xsum_length n)
{
  if (xsum_debug) printf("LARGE ADD_DOT OF %ld VALUES\n",(long)n);

  if (n == 0) return;

# if OPT_LARGE_DOT
  { 
    /* Version that's been manually optimized:  Loop unrolled, pre-fetch
       attempted, branches eliminated, ... */

    union fpunion u1, u2;
    int count1, count2;
    xsum_expint ix1, ix2;
    const xsum_flt *v1, *v2;
    xsum_flt f1, f2;
    xsum_length m;

    v1 = vec1; v2 = vec2;
    f1 = *v1;  f2 = *v2;

    /* Unrolled loop processing two products each time around.  The loop is
       done as two nested loops, arranged so that the inner one will have
       no branches except for the one looping back.  This is achieved by
       a trick for combining three tests for negativity into one.  The
       last one or two products are not done here, so that the pre-fetching
       will not go past the end of the array (which would probably be OK,
       but is technically not allowed). */

    m = n-3; /* leave out last one or two, terminate when negative, for trick */
    while (m >= 0)
    { 
      /* Loop processing two products at a time until we're done, or until 
         one (or both) of them result in a chunk needing to be processed. 
         Updates are done here for both of these chunks, even though it is not
         yet known whether these updates ought to have been done.  We hope
         this allows for better memory pre-fetch and instruction scheduling. */

      do
      { u1.fltv = f1 * f2;
        v1 += 1;  v2 += 1;
        f1 = *v1; f2 = *v2;
        u2.fltv = f1 * f2;

        ix1 = u1.uintv >> XSUM_MANTISSA_BITS;
        count1 = lacc->count[ix1] - 1;
        lacc->count[ix1] = count1;
        lacc->chunk[ix1] += u1.uintv;

        v1 += 1;  v2 += 1;
        f1 = *v1; f2 = *v2;

        ix2 = u2.uintv >> XSUM_MANTISSA_BITS;
        count2 = lacc->count[ix2] - 1;
        lacc->count[ix2] = count2;
        lacc->chunk[ix2] += u2.uintv;

        m -= 2;

      } while (((xsum_length)count1 | (xsum_length)count2 | m) >= 0);
           /* ... equivalent to while (count1 >= 0 && count2 >= 0 && m >= 0) */

      /* See if we were actually supposed to update these chunks.  If not,
         back out the changes and then process the chunks as they ought to
         have been processed. */

      if (count1 < 0 || count2 < 0)
      { lacc->count[ix2] = count2 + 1;
        lacc->chunk[ix2] -= u2.uintv;
        if (count1 < 0)
        { lacc->count[ix1] = count1 + 1;
          lacc->chunk[ix1] -= u1.uintv;
          xsum_large_add_value_inf_nan (lacc, ix1, u1.uintv);
          count2 = lacc->count[ix2] - 1;
        }
        if (count2 < 0)
        { xsum_large_add_value_inf_nan (lacc, ix2, u2.uintv);
        }
        else
        { lacc->count[ix2] = count2;
          lacc->chunk[ix2] += u2.uintv;
        }
      }

    }

    /* Process the last one or two products, without pre-fetching. */

    m += 3;
    for (;;)
    { u1.fltv = f1 * f2;
      ix1 = u1.uintv >> XSUM_MANTISSA_BITS;
      count1 = lacc->count[ix1] - 1;
      if (count1 < 0)
      { xsum_large_add_value_inf_nan (lacc, ix1, u1.uintv);
      }
      else
      { lacc->count[ix1] = count1;
        lacc->chunk[ix1] += u1.uintv;
      }
      m -= 1;
      if (m == 0) break;
      v1 += 1;  v2 += 1;
      f1 = *v1; f2 = *v2;
    }
  }
# else
  {
    /* Version not manually optimized - maybe the compiler can do better. */

    union fpunion u;
    xsum_lcount count;
    xsum_expint ix;

    do
    {
      /* Fetch the next numbers, multiply them, and convert the result to
         integer form in u.uintv. */

      u.fltv = *vec1 * *vec2;
      vec1 += 1; vec2 += 1;

      /* Isolate the upper sign+exponent bits that index the chunk. */

      ix = u.uintv >> XSUM_MANTISSA_BITS;

      /* Find the count for this chunk, and subtract one. */

      count = lacc->count[ix] - 1;

      if (count < 0)
      { 
        /* If the decremented count is negative, it's either a special 
           Inf/NaN chunk (in which case count will stay at -1), or one that 
           needs to be transferred to the small accumulator, or one that
           has never been used before and needs to be initialized. */

        xsum_large_add_value_inf_nan (lacc, ix, u.uintv);
      }
      else
      { 
        /* Store the decremented count of additions allowed before transfer,
           and add this value to the chunk. */

        lacc->count[ix] = count;
        lacc->chunk[ix] += u.uintv;
      }
      n -= 1;
    } while (n > 0);
  }
# endif
}


/* ADD A LARGE ACCUMULATOR TO ANOTHER LARGE ACCUMULATOR.  The first argument
   is the destination, which is modified.  The second is the accumulator to
   add, which may also be modified, but should still represent the same
   number.  Source and destination must be different. */

void xsum_large_add_accumulator (xsum_large_accumulator *restrict dst_lacc, 
                                 xsum_large_accumulator *restrict src_lacc)
{
  if (xsum_debug) printf("Adding accumulator to a large accumulator\n");
  if (dst_lacc == src_lacc) abort();

  xsum_large_transfer_to_small (src_lacc);
  xsum_small_add_accumulator (&dst_lacc->sacc, &src_lacc->sacc);
}


/* RETURN RESULT OF ROUNDING A LARGE ACCUMULATOR.  Rounding mode is to nearest,
   with ties to even.  

   This is done by adding all the chunks in the large accumulator to the
   small accumulator, and then calling its rounding procedure. */

xsum_flt xsum_large_round (xsum_large_accumulator *restrict lacc)
{
  if (xsum_debug) printf("Rounding large accumulator\n");

  xsum_large_transfer_to_small (lacc);

  return xsum_small_round (&lacc->sacc);
}


/* TRANSFER NUMBER FROM A LARGE ACCUMULATOR TO A SMALL ACCUMULATOR. */

void xsum_large_to_small_accumulator (xsum_small_accumulator *restrict sacc, 
                                      xsum_large_accumulator *restrict lacc)
{
  if (xsum_debug) printf("Transferring from large to small accumulator\n");
  xsum_large_transfer_to_small (lacc);
  *sacc = lacc->sacc;
}


/* TRANSFER NUMBER FROM A SMALL ACCUMULATOR TO A LARGE ACCUMULATOR. */

void xsum_small_to_large_accumulator (xsum_large_accumulator *restrict lacc, 
                                      xsum_small_accumulator *restrict sacc)
{
  if (xsum_debug) printf("Transferring from small to large accumulator\n");
  xsum_large_init_chunks (lacc);
  lacc->sacc = *sacc;
}


/* ------------------- ROUTINES FOR NON-EXACT SUMMATION --------------------- */


/* SUM A VECTOR WITH DOUBLE FP ACCUMULATOR. */

xsum_flt xsum_sum_double (const xsum_flt *restrict vec, 
                          xsum_length n)
{ double s;
  xsum_length j;
  s = 0.0;
# if OPT_SIMPLE_SUM
  { for (j = 3; j < n; j += 4) 
    { s += vec[j-3]; 
      s += vec[j-2]; 
      s += vec[j-1]; 
      s += vec[j]; 
    }
    for (j = j-3; j < n; j++)
    { s += vec[j];
    }
  }
# else
  { for (j = 0; j < n; j++) 
    { s += vec[j];
    }
  }
# endif
  return (xsum_flt) s;
}


/* SUM A VECTOR WITH FLOAT128 ACCUMULATOR. */

#ifdef FLOAT128

#include <quadmath.h>

xsum_flt xsum_sum_float128 (const xsum_flt *restrict vec, 
                            xsum_length n)
{ __float128 s;
  xsum_length j;
  s = 0.0;
  for (j = 0; j < n; j++) 
  { s += vec[j];
  }
  return (xsum_flt) s;
}

#endif


/* SUM A VECTOR WITH DOUBLE FP, NOT IN ORDER. */

xsum_flt xsum_sum_double_not_ordered (const xsum_flt *restrict vec, 
                                      xsum_length n)
{ double s1, s2;
  xsum_length j;
  s1 = s2 = 0.0;
  for (j = 1; j < n; j += 2) 
  { s1 += vec[j-1];
    s2 += vec[j];
  }
  if (j == n)
  { s1 += vec[j-1];
  }
  return (xsum_flt) (s1+s2);
}


/* SUM A VECTOR WITH KAHAN'S METHOD. */

xsum_flt xsum_sum_kahan (const xsum_flt *restrict vec, 
                         xsum_length n)
{ double s, t, c, y;
  xsum_length j;
  s = 0.0;
  c = 0.0;
# if OPT_KAHAN_SUM
  { for (j = 1; j < n; j += 2) 
    { y = vec[j-1] - c;
      t = s;
      s += y;
      c = (s - t) - y;
      y = vec[j] - c;
      t = s;
      s += y;
      c = (s - t) - y;
    }
    for (j = j-1; j < n; j++) 
    { y = vec[j] - c;
      t = s;
      s += y;
      c = (s - t) - y;
    }
  }
# else
  { for (j = 0; j < n; j++) 
    { y = vec[j] - c;
      t = s;
      s += y;
      c = (s - t) - y;
    }
  }
# endif
  return (xsum_flt) s;
}


/* SQUARED NORM OF A VECTOR WITH DOUBLE FP ACCUMULATOR. */

xsum_flt xsum_sqnorm_double (const xsum_flt *restrict vec, 
                             xsum_length n)
{ double s;
  xsum_length j;

  s = 0.0;
# if OPT_SIMPLE_SQNORM
  { double a, b, c, d;
    for (j = 3; j < n; j += 4) 
    { a = vec[j-3]; 
      b = vec[j-2]; 
      c = vec[j-1]; 
      d = vec[j];
      s += a*a;
      s += b*b;
      s += c*c;
      s += d*d;
    }
    for (j = j-3; j < n; j++)
    { a = vec[j];
      s += a*a;
    }
  }
# else
  { double a;
    for (j = 0; j < n; j++) 
    { a = vec[j];
      s += a*a;
    }
  }
# endif
  return (xsum_flt) s;
}


/* SQUARED NORM OF A VECTOR WITH DOUBLE FP, NOT IN ORDER. */

xsum_flt xsum_sqnorm_double_not_ordered (const xsum_flt *restrict vec, 
                                         xsum_length n)
{ double s1, s2, a, b;
  xsum_length j;
  s1 = s2 = 0.0;
  for (j = 1; j < n; j += 2) 
  { a = vec[j-1];
    b = vec[j];
    s1 += a*a;
    s2 += b*b;
  }
  if (j == n)
  { a = vec[j-1];
    s1 += a*a;
  }
  return (xsum_flt) (s1+s2);
}


/* DOT PRODUCT OF VECTORS WITH DOUBLE FP ACCUMULATOR. */

xsum_flt xsum_dot_double (const xsum_flt *vec1, 
                          const xsum_flt *vec2, 
                          xsum_length n)
{ double s;
  xsum_length j;

  s = 0.0;
# if OPT_SIMPLE_DOT
  { for (j = 3; j < n; j += 4) 
    { s += vec1[j-3] * vec2[j-3];
      s += vec1[j-2] * vec2[j-2];
      s += vec1[j-1] * vec2[j-1];
      s += vec1[j] * vec2[j];
    }
    for (j = j-3; j < n; j++)
    { s += vec1[j] * vec2[j];
    }
  }
# else
  { for (j = 0; j < n; j++) 
    { s += vec1[j] * vec2[j];
    }
  }
# endif
  return (xsum_flt) s;
}


/* DOT PRODUCT OF VECTORS WITH DOUBLE FP, NOT IN ORDER. */

xsum_flt xsum_dot_double_not_ordered (const xsum_flt *vec1, 
                                      const xsum_flt *vec2, 
                                      xsum_length n)
{ double s1, s2;
  xsum_length j;
  s1 = s2 = 0.0;
  for (j = 1; j < n; j += 2) 
  { s1 += vec1[j-1] * vec2[j-1];
    s2 += vec1[j] * vec2[j];
  }
  if (j == n)
  { s1 += vec1[j-1] * vec2[j-1];
  }
  return (xsum_flt) (s1+s2);
}


/* ------------------------- DEBUGGING ROUTINES ----------------------------- */


/* DISPLAY A SMALL ACCUMULATOR. */

void xsum_small_display (xsum_small_accumulator *restrict sacc)
{
  int i, dots;
  printf("Small accumulator:");
  if (sacc->Inf) 
  { printf (" %cInf", sacc->Inf>0 ? '+' : '-');
  }
  if (sacc->NaN)
  { printf (" NaN (%llx)", (long long) sacc->NaN);
  }
  printf("\n");
  dots = 0;
  for (i = XSUM_SCHUNKS-1; i >= 0; i--)
  { if (sacc->chunk[i] == 0)
    { if (!dots) printf("            ...\n");
      dots = 1;
    }
    else
    { printf ("%5d %5d ", i, (int)
       ((i<<XSUM_LOW_EXP_BITS) - XSUM_EXP_BIAS - XSUM_MANTISSA_BITS));
      pbinary_int64 ((int64_t) sacc->chunk[i] >> 32, XSUM_SCHUNK_BITS-32);
      printf(" ");
      pbinary_int64 ((int64_t) sacc->chunk[i] & 0xffffffff, 32);
      printf ("\n");
      dots = 0;
    }
  }
  printf("\n");
}


/* RETURN NUMBER OF CHUNKS IN USE IN SMALL ACCUMULATOR. */

int xsum_small_chunks_used (xsum_small_accumulator *restrict sacc)
{
  int i, c;
  c = 0;
  for (i = 0; i < XSUM_SCHUNKS; i++)
  { if (sacc->chunk[i] != 0)
    { c += 1;
    }
  }
  return c;
}


/* DISPLAY A LARGE ACCUMULATOR. */

void xsum_large_display (xsum_large_accumulator *restrict lacc)
{
  int i, dots;
  printf("Large accumulator:\n");
  dots = 0;
  for (i = XSUM_LCHUNKS-1; i >= 0; i--)
  { if (lacc->count[i] < 0)
    { if (!dots) printf("            ...\n");
      dots = 1;
    }
    else
    { printf ("%c%4d %5d ", i & 0x800 ? '-' : '+', i & 0x7ff, lacc->count[i]);
      pbinary_int64 ((int64_t) lacc->chunk[i] >> 32, XSUM_LCHUNK_BITS-32);
      printf(" ");
      pbinary_int64 ((int64_t) lacc->chunk[i] & 0xffffffff, 32);
      printf ("\n");
      dots = 0;
    }
  }
  printf("\nWithin large accumulator:  ");
  xsum_small_display (&lacc->sacc);

}


/* RETURN NUMBER OF CHUNKS IN USE IN LARGE ACCUMULATOR. */

int xsum_large_chunks_used (xsum_large_accumulator *restrict lacc)
{
  int i, c;
  c = 0;
  for (i = 0; i < XSUM_LCHUNKS; i++)
  { if (lacc->count[i] >= 0)
    { c += 1;
    }
  }
  return c;
}
