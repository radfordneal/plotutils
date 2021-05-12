/* TIME MEASUREMENT PROGRAM FOR FUNCTIONS FOR EXACT SUMMATION. */

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


/* Program to perform timing tests of exact and non-exact summation methods,
   possibly with permuted data.

   Run with a command of the form:

       xsum-time [ task ] [ method ] N M R [ data ... ] [ "inf" ] [ "perm" ]

   Here, N is the size of the vectors that are summed, M is the number
   of such vectors, and R is the number of times these M vectors of
   size N are summed, by each method.  All tasks are done, unless a
   task is specified, in which case only it is run.  All available
   methods are run, unless a method is specified, in which case only
   it is run.  The three tasks are "sum", "norm", and "dot".  Possible
   methods are "sacc", "lacc", "iFast", "OnlineExact", "float128",
   "double", and "udouble", some of which may not be available
   depending on how xsum-time was compiled.

   After running these tests for summing vector elements, the sum of the
   squares is also computed, and then the dot product of each vector
   with a corresponding vector in a different set of M vectors.

   The data in each of the vectors is as specified after R, with any
   remaining elements filled in randomly, differently for each of the
   M vectors.  This random data sums to zero, since it consists of two
   copies of a set of numbers, with the second copy negated (with an
   additional zero if the number of elements is odd).  In addition, if
   "inf" is specified, every eigth element (before possible
   permutation) is set to infinity (so the result will also be
   infinity).  If "perm" is not specified, the second negated copy
   follows the first, with mirroring.  If "perm" is specified, the
   order of all elements is randomized.

   This program can be compiled to compare with Zhu & Hayes' iFastSum
   and ExactSum methods, if the source for those is present.  The -DZHU
   option must be given to the compiler for this to happend.

   This program can be compiled to compare with 128-bit floating point
   arithmetic using the gcc __float128 type.  The -DFLOAT128 option
   must be given to the compiler for this to happen.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "xsum.h"

#ifdef ZHU
#include "ExactSum.h"
#endif

#define START_CLOCK ( start_clock = clock() )
#define END_CLOCK ( clock_dur = (double)(clock()-start_clock)/CLOCKS_PER_SEC )

int different (double a, double b)
{ 
  return isnan(a) != isnan(b) || !isnan(a) && !isnan(b) && a != b;
}

int main (int argc, char **argv)
{
  clock_t start_clock;
  double clock_dur;
  xsum_flt result_s, result_l, result_d, result_dno, result_k;
# ifdef ZHU
  xsum_flt result_iFast, result_Online;
  ExactSum mysum;
  xsum_flt *ai;
# endif
# ifdef FLOAT128
  xsum_flt result_128;
# endif
  int used_small, used_large;
  xsum_small_accumulator sacc;
  xsum_large_accumulator lacc;
  xsum_flt *a, *a2;
  int N, M, R;
  int i, j, k;
  int ndata, n;
  char **data;
  char *task;
  char *method;
  int perm, inf;

  task = 0;
  if (argc>1 && (strcmp(argv[1],"sum")==0 
                  || strcmp(argv[1],"norm")==0 || strcmp(argv[1],"dot")==0))
  { task = argv[1];
    argv += 1;
    argc -= 1;
  }

  method = 0;
  if (argc>1 && (argv[1][0]>='a' && argv[1][0]<='z'
                  || argv[1][0]>='A' && argv[1][0]<='Z'))
  { method = argv[1];
    argv += 1;
    argc -= 1;
  }

  perm = 0;
  if (argc>1 && strcmp(argv[argc-1],"perm")==0)
  { perm = 1;
    argc -= 1;
  }

  inf = 0;
  if (argc>1 && strcmp(argv[argc-1],"inf")==0)
  { inf = 1;
    argc -= 1;
  }

  ndata = argc-4;

  if (argc<4 || (N=atoi(argv[1]))<1 || ndata>N
             || (M=atoi(argv[2]))<1 || (R=atoi(argv[3]))<1)
  { fprintf(stderr,
     "Usage: xsum-time [ task ] [ method ] N M R [ data ... ] [ \"inf\" ] [ \"perm\" ]\n");
    exit(1);
  }

  data = argv+4;

  /* On an Intel machine, set the 387 FPU to do double rounding, in order
     to get correct IEEE 64-bit floating point.  (Only relevant if SSE2
     instructions not used instead of FPU.)  This will disable higher
     precision for long double, however! */

# ifdef DOUBLE
  { unsigned int mode = 0x27f;
    __asm__ ("fldcw %0" : : "m" (*&mode));
  }
# endif

  printf("\nTIMING TESTS\n\n");
  printf("N = %d, M = %d, R = %d",N,M,R);
  if (ndata > 0 || inf)
  { printf("  data:");
    for (j = 0; j < ndata; j++) printf(" %s",data[j]);
    if (inf) printf(" inf"); 
  }
  printf("\n");
 
  a = (xsum_flt *) calloc (2*N*M, sizeof *a);  /* Put a and a2 into one block */
  a2 = a + N*M;        /* to suppress possible variation in cache performance */

# ifdef ZHU
  ai = (xsum_flt *) calloc (N, sizeof *ai);
# endif

  n = N - ndata;
  for (i = 0; i < M; i++)
  { for (j = 0; j < ndata; j++) a[i*N+j] = atof(data[j]);
    if (n > 0)
    { int64_t rnd;
      a[ndata+i*N+n/2] = 0;
      rnd = 2345678;
      for (j = 0; j < n/2; j++)
      { rnd = (rnd*8192) % 67101323;/* from "Multiplicative congruential pseudo-
                                   random number generators", Downham&Roberts */
        a[ndata+i*N+j] = exp (30 * (double)rnd / 67101323);
        rnd = (rnd*8192) % 67101323;
        a[ndata+i*N+j] *= (double)rnd / 67101323;
        a[ndata+i*N+n-1-j] = -a[ndata+i*N+j];
        if (inf && j%8 == 0) 
        { a[ndata+i*N+n-1-j] = a[ndata+i*N+j] = 1.0/0.0;
        }
      }
      if (perm)
      { for (j = 0; j < N; j++)
        { xsum_flt t;
          int w;
          rnd = (rnd*8192) % 67101323;
          w = j + rnd % (N-j);
          t = a[w];
          a[w] = a[j];
          a[j] = t;
        }
      }
    }
  }

  for (i = 0; i < N*M; i++) a2[i] = a[i];

  if (task==0 || strcmp(task,"sum")==0)
  {
    printf("\nVECTOR SUM\n\n");

    if (method==0 || strcmp(method,"sacc")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { xsum_small_init(&sacc);
          xsum_small_addv(&sacc,a+k*N,N);
          result_s = xsum_small_round(&sacc);
        }
      }
      END_CLOCK;
      printf("Small accumulator:  %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_s, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"lacc")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { xsum_large_init(&lacc);
          xsum_large_addv(&lacc,a+k*N,N);
          result_l = xsum_large_round(&lacc);
        }
      }
      END_CLOCK;
      printf("Large accumulator:  %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_l, clock_dur, clock_dur*1e9/R/N/M);
    }

#   ifdef ZHU

    if (method==0 || strcmp(method,"iFast")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { memcpy (ai, a+k*N, N * sizeof *ai);
          result_iFast = mysum.iFastSum(ai-1,N);
        }
      }
      END_CLOCK;
      printf("iFastSum result:    %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_iFast, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"OnlineExact")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_Online = mysum.OnlineExactSum(a+k*N-1,N);
        }
      }
      END_CLOCK;
      printf("OnlineExact result: %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_Online, clock_dur, clock_dur*1e9/R/N/M);
    }

#   endif

#   ifdef FLOAT128

    if (method==0 || strcmp(method,"float128")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_128 = xsum_sum_float128(a+k*N,N);
        }
      }
      END_CLOCK;
      printf("Float 128 result:   %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_128, clock_dur, clock_dur*1e9/R/N/M);
    }

#   endif

    if (method==0 || strcmp(method,"Kahan")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_k = xsum_sum_kahan(a+k*N,N);
        }
      }
      END_CLOCK;
      printf("Kahan sum result:   %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_k, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"double")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_d = xsum_sum_double(a+k*N,N);
        }
      }
      END_CLOCK;
      printf("Double result:      %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_d, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"udouble")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_dno = xsum_sum_double_not_ordered(a+k*N,N);
        }
      }
      END_CLOCK;
      printf("Double not ordered: %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_dno, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (ndata <= 2)
    { double correct;
      int i;
      if (inf && n/2 > 0)
      { correct = 1.0/0.0;
      }
      else
      { correct = 0.0;
        for (i = 0; i < ndata; i++) correct += atof(data[i]);
      }
      if ((method==0 || strcmp(method,"sacc")==0) 
                         && different(result_s,correct))
      { printf("RESULT USING SMALL ACCUMULATOR IS WRONG\n");
      }
      if ((method==0 || strcmp(method,"lacc")==0) 
                         && different(result_l,correct))
      { printf("RESULT USING LARGE ACCUMULATOR IS WRONG\n");
      }
    }

    if ((method==0 || strcmp(method,"sacc")==0) 
          && result_s != (double) xsum_small_round(&sacc))
    { printf ("RESULT DIFFERS AFTER ROUNDING SMALL ACCUMULATOR TWICE\n");
    }

    if ((method==0 || strcmp(method,"lacc")==0) 
          &&result_l != (double) xsum_large_round(&lacc))
    { printf ("RESULT DIFFERS AFTER ROUNDING LARGE ACCUMULATOR TWICE\n");
    }

    if (method==0 && result_s != result_l)
    { printf("RESULTS DIFFER FOR SMALL AND LARGE ACCUMULATORS\n");
    }

#   ifdef ZHU

    if (method==0 && result_iFast != result_l)
    { printf("RESULTS DIFFER FOR IFASTSUM AND LARGE ACCUMULATOR\n");
    }

    if (method==0 && result_Online != result_l)
    { printf("RESULTS DIFFER FOR ONLINEEXACT AND LARGE ACCUMULATOR\n");
    }

#   endif

    if (method==0 && N <= 2 && result_s != result_d)
    { printf("RESULTS DIFFER FOR DOUBLE AND SMALL ACCUMULATOR\n");
    }

    if (method==0 && N <= 2 && result_l != result_d)
    { printf("RESULTS DIFFER FOR DOUBLE AND LARGE ACCUMULATOR\n");
    }

    printf("\n");
    if ((method==0 || strcmp(method,"sacc")==0) && !perm)
    { xsum_small_init(&sacc); /* Get used counts half-way, before cancels to 0*/
      xsum_small_addv(&sacc,a,N/2);
      used_small = xsum_small_chunks_used(&sacc);
      printf("Small accumulator chunks used: %d\n", used_small);
    }

    if (method==0 || strcmp(method,"lacc")==0)
    { used_large = xsum_large_chunks_used(&lacc);
      printf("Large accumulator chunks used: %d\n", used_large);
    }
  }

  if (task==0 || strcmp(task,"norm")==0)
  { 
    printf("\nVECTOR NORM\n\n");

    if (method==0 || strcmp(method,"sacc")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { xsum_small_init(&sacc);
          xsum_small_add_sqnorm(&sacc,a+k*N,N);
          result_s = xsum_small_round(&sacc);
        }
      }
      END_CLOCK;
      printf("Small accumulator:  %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_s, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"lacc")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { xsum_large_init(&lacc);
          xsum_large_add_sqnorm(&lacc,a+k*N,N);
          result_l = xsum_large_round(&lacc);
        }
      }
      END_CLOCK;
      printf("Large accumulator:  %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_l, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"double")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_d = xsum_sqnorm_double(a+k*N,N);
        }
      }
      END_CLOCK;
      printf("Double result:      %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_d, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"udouble")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_dno = xsum_sqnorm_double_not_ordered(a+k*N,N);
        }
      }
      END_CLOCK;
      printf("Double not ordered: %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_dno, clock_dur, clock_dur*1e9/R/N/M);
    }

    if ((method==0 || strcmp(method,"sacc")==0)
          && result_s != (double) xsum_small_round(&sacc))
    { printf ("RESULT DIFFERS AFTER ROUNDING SMALL ACCUMULATOR TWICE\n");
    }

    if ((method==0 || strcmp(method,"lacc")==0)
          && result_l != (double) xsum_large_round(&lacc))
    { printf ("RESULT DIFFERS AFTER ROUNDING LARGE ACCUMULATOR TWICE\n");
    }

    if (method==0 && result_s != result_l)
    { printf("RESULTS DIFFER FOR SMALL AND LARGE ACCUMULATORS\n");
    }

    if (method==0 && N <= 2 && result_s != result_d)
    { printf("RESULTS DIFFER FOR DOUBLE AND SMALL ACCUMULATOR\n");
    }

    if (method==0 && N <= 2 && result_l != result_d)
    { printf("RESULTS DIFFER FOR DOUBLE AND LARGE ACCUMULATOR\n");
    }

    printf("\n");

    if (method==0 || strcmp(method,"sacc")==0)
    { xsum_small_init(&sacc);
      xsum_small_add_sqnorm(&sacc,a,N);
      used_small = xsum_small_chunks_used(&sacc);
      printf("Small accumulator chunks used: %d\n", used_small);
    }

    if (method==0 || strcmp(method,"lacc")==0)
    { used_large = xsum_large_chunks_used(&lacc);
      printf("Large accumulator chunks used: %d\n", used_large);
    }
  }

  if (task==0 || strcmp(task,"dot")==0)
  {
    printf("\nVECTOR DOT PRODUCT\n\n");

    if (method==0 || strcmp(method,"sacc")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { xsum_small_init(&sacc);
          xsum_small_add_dot(&sacc,a+k*N,a2+k*N,N);
          result_s = xsum_small_round(&sacc);
        }
      }
      END_CLOCK;
      printf("Small accumulator:  %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_s, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"lacc")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { xsum_large_init(&lacc);
          xsum_large_add_dot(&lacc,a+k*N,a2+k*N,N);
          result_l = xsum_large_round(&lacc);
        }
      }
      END_CLOCK;
      printf("Large accumulator:  %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_l, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"double")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_d = xsum_dot_double(a+k*N,a2+k*N,N);
        }
      }
      END_CLOCK;
      printf("Double result:      %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_d, clock_dur, clock_dur*1e9/R/N/M);
    }

    if (method==0 || strcmp(method,"udouble")==0)
    { START_CLOCK;
      for (j = 0; j < R; j++)
      { for (k = 0; k < M; k++)
        { result_dno = xsum_dot_double_not_ordered(a+k*N,a2+k*N,N);
        }
      }
      END_CLOCK;
      printf("Double not ordered: %+.16le  time: %7.3lf s, %7.2lf ns/term\n",
             result_dno, clock_dur, clock_dur*1e9/R/N/M);
    }

    if ((method==0 || strcmp(method,"sacc")==0)
          && result_s != (double) xsum_small_round(&sacc))
    { printf ("RESULT DIFFERS AFTER ROUNDING SMALL ACCUMULATOR TWICE\n");
    }

    if ((method==0 || strcmp(method,"lacc")==0)
          && result_l != (double) xsum_large_round(&lacc))
    { printf ("RESULT DIFFERS AFTER ROUNDING LARGE ACCUMULATOR TWICE\n");
    }

    if (method==0 && result_s != result_l)
    { printf("RESULTS DIFFER FOR SMALL AND LARGE ACCUMULATORS\n");
    }

    if (method==0 && N <= 2 && result_s != result_d)
    { printf("RESULTS DIFFER FOR DOUBLE AND SMALL ACCUMULATOR\n");
    }

    if (method==0 && N <= 2 && result_l != result_d)
    { printf("RESULTS DIFFER FOR DOUBLE AND LARGE ACCUMULATOR\n");
    }

    printf("\n");

    if (method==0 || strcmp(method,"sacc")==0)
    { xsum_small_init(&sacc);
      xsum_small_add_dot(&sacc,a,a2,N);
      used_small = xsum_small_chunks_used(&sacc);
      printf("Small accumulator chunks used: %d\n", used_small);
    }

    if (method==0 || strcmp(method,"lacc")==0)
    { used_large = xsum_large_chunks_used(&lacc);
      printf("Large accumulator chunks used: %d\n", used_large);
    }

    printf("\n");
  }

  return 0;
}
