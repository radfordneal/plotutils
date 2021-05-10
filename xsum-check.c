/* AUTOMATIC CORRECTNESS CHECKS FOR FUNCTIONS FOR EXACT SUMMATION. */

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


/* Program to check correctness of the small and large superaccumulator
   methods.  May be passed a -e argument to echo details of tests.  
   If -d is given instead, it both echos and prints debug information.
   If -dL# is given, it prints debug information only for test # in
   section L. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xsum.h"
#include "pbinary.h"

#define pow2_16 (1.0 / (1 << 16))
#define pow2_32 (pow2_16 * pow2_16)
#define pow2_64 (pow2_32 * pow2_32)
#define pow2_128 (pow2_64 * pow2_64)
#define pow2_256 (pow2_128 * pow2_128)
#define pow2_512 (pow2_256 * pow2_256)
#define pow2_1024 (pow2_512 * pow2_512)

#define pow2_52 (1.0 / (1 << 22) / (1 << 30))

#define Lnormal (2*((.5/pow2_1024)-(.25/pow2_1024)*pow2_52))
#define Snormal (4*pow2_1024)
#define Ldenorm (4*pow2_1024 - 4*pow2_1024*pow2_52)
#define Sdenorm (4*pow2_1024*pow2_52)

double zero = 0.0;

/* Tests with one term.  Answer should be the same as the term. */

xsum_flt one_term[] = {

  1.0,             /* Some unexceptional examples of normal numbers */
  -1.0,
  0.1,
  -0.1,
  3.1,
  -3.1,
  2.3e10,
  -2.3e10,
  3.2e-10,
  -3.2e-10,
  123e123,
  -123e123,
  54.11e-150,
  -54.11e-150,
  2*((.5/pow2_128)-(.25/pow2_128)*pow2_52),  /* Mantissa all 1s */
  -2*((.5/pow2_128)+(.25/pow2_128)*pow2_52),
  Lnormal,                                /* Largest normal number */
  -Lnormal,
  Snormal,                                /* Smallest normal number */
  -Snormal,
  Ldenorm,                                /* Largest denormalized number */
  -Ldenorm,
  Sdenorm,                                /* Smallest denormalized number > 0 */
  -Sdenorm,
  1.23e-309,                              /* Other denormalized numbers */
  -1.23e-309,
  4.57e-314,
  -4.57e-314,
  9.7e-322,
  -9.7e-322,
  Sdenorm/pow2_64/2,
  -Sdenorm/pow2_64/2,

0 };

/* Tests with two terms.  Answer should match ordinary floating point add. */

xsum_flt two_term[] = {

1.0, 2.0,         /* Unexceptional adds of normal numbers */
-1.0, -2.0,
0.1, 12.2,
-0.1, -12.2,
12.1, -11.3,
-12.1, 11.3,
11.3, -12.1,
-11.3, 12.1,
1.234567e14, 9.87654321,
-1.234567e14, -9.87654321,
1.234567e14, -9.87654321,
-1.234567e14, 9.87654321,
3.1e200, 1.7e-100,  /* Smaller term should disappear */
3.1e200, -1.7e-100,
-3.1e200, 1.7e-100,
-3.1e200, -1.7e-100,
1.7e-100, 3.1e200,
1.7e-100, -3.1e200,
-1.7e-100, 3.1e200,
-1.7e-100, -3.1e200,
1, pow2_52,       /* Test rounding */
-1, -pow2_52,
1, pow2_52/2,
-1, -pow2_52/2,
1, pow2_52/2+pow2_52/4096,
-1, -pow2_52/2-pow2_52/4096,
1, pow2_52/2+pow2_52/(1<<30)/(1<<10),
-1, -pow2_52/2-pow2_52/(1<<30)/(1<<10),
1, pow2_52/2-pow2_52/4096,
-1, -pow2_52/2+pow2_52/4096,
1 + pow2_52, pow2_52/2,
1 + pow2_52, pow2_52/2 - pow2_52*pow2_52,
-(1 + pow2_52), -pow2_52/2,
-(1 + pow2_52), -(pow2_52/2 - pow2_52*pow2_52),
Sdenorm, 7.1,              /* Adds with denormalized numbers */
Sdenorm, -7.1,
-Sdenorm, -7.1,
-Sdenorm, 7.1,
7.1, Sdenorm,
-7.1, Sdenorm,
-7.1, -Sdenorm,
7.1, -Sdenorm,
Ldenorm, Sdenorm,
Ldenorm, -Sdenorm,
-Ldenorm, Sdenorm,
-Ldenorm, -Sdenorm,
Sdenorm, Sdenorm,
Sdenorm, -Sdenorm,
-Sdenorm, Sdenorm,
-Sdenorm, -Sdenorm,
Ldenorm, Snormal,
Snormal, Ldenorm,
-Ldenorm, -Snormal,
-Snormal, -Ldenorm,
4.57e-314, 9.7e-322,
-4.57e-314, 9.7e-322,
4.57e-314, -9.7e-322,
-4.57e-314, -9.7e-322,
4.57e-321, 9.7e-322,
-4.57e-321, 9.7e-322,
4.57e-321, -9.7e-322,
-4.57e-321, -9.7e-322,
2.0, -2.0*(1+pow2_52),
Lnormal, Lnormal,              /* Overflow */
-Lnormal, -Lnormal,
Lnormal, Lnormal*pow2_52/2,
-Lnormal, -Lnormal*pow2_52/2,
1.0/0.0, 123,                  /* Infinity / NaN */
-1.0/0.0, 123,
1.0/0.0, -1.0/0.0,
0.0/0.0, 123,
123, 0.0/0.0,
0 };

/* Tests with three terms.  Answers are given here as a fourth number,
   some computed/verified using Rmpfr in check.r. */

xsum_flt three_term[] = {
Lnormal, Sdenorm, -Lnormal, Sdenorm,
-Lnormal, Sdenorm, Lnormal, Sdenorm,
Lnormal, -Sdenorm, -Lnormal, -Sdenorm,
-Lnormal, -Sdenorm, Lnormal, -Sdenorm,
Sdenorm, Snormal, -Sdenorm, Snormal,
-Sdenorm, -Snormal, Sdenorm, -Snormal,
12345.6, Snormal, -12345.6, Snormal,
12345.6, -Snormal, -12345.6, -Snormal,
12345.6, Ldenorm, -12345.6, Ldenorm,
12345.6, -Ldenorm, -12345.6, -Ldenorm,
2.0, -2.0*(1+pow2_52), pow2_52/8, -2*pow2_52+pow2_52/8,
1.0, 2.0, 3.0, 6.0,
12.0, 3.5, 2.0, 17.5,
3423.34e12, -93.431, -3432.1e11, 3080129999999906.5,
432457232.34, 0.3432445, -3433452433, -3000995200.3167553,
0 };

/* Tests with ten terms.  Answers are given here as an eleventh number,
   some computed/verified using Rmpfr in check.r. */

xsum_flt ten_term[] = {
Lnormal, Lnormal, Lnormal, Lnormal, Lnormal, Lnormal, -Lnormal, -Lnormal, -Lnormal, -Lnormal, 1.0/0.0,
-Lnormal, -Lnormal, -Lnormal, -Lnormal, -Lnormal, -Lnormal, Lnormal, Lnormal, Lnormal, Lnormal, -1.0/0.0,
Lnormal, Lnormal, Lnormal, Lnormal, 0.125, 0.125, -Lnormal, -Lnormal, -Lnormal, -Lnormal, 0.25,
2.0*(1+pow2_52), -2.0, -pow2_52, -pow2_52, 0, 0, 0, 0, 0, 0, 0,
1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1111111111e0,
-1e0, -1e1, -1e2, -1e3, -1e4, -1e5, -1e6, -1e7, -1e8, -1e9, -1111111111e0,
1.234e88, -93.3e-23, 994.33, 1334.3, 457.34, -1.234e88, 93.3e-23, -994.33, -1334.3, -457.34, 0,
1., -23., 456., -78910., 1112131415., -161718192021., 22232425262728., -2930313233343536., 373839404142434445., -46474849505152535455., -46103918342424313856.,
2342423.3423, 34234.450, 945543.4, 34345.34343, 1232.343, 0.00004343, 43423.0, -342344.8343, -89544.3435, -34334.3, 2934978.4009734304,
0.9101534, 0.9048397, 0.4036596, 0.1460245, 0.2931254, 0.9647649, 0.1125303, 0.1574193, 0.6522300, 0.7378597, 5.2826068,
428.366070546, 707.3261930632, 103.29267289, 9040.03475821, 36.2121638, 19.307901408, 1.4810709160, 8.077159101, 1218.907244150, 778.068267017, 12341.0735011012,
1.1e-322, 5.3443e-321, -9.343e-320, 3.33e-314, 4.41e-322, -8.8e-318, 3.1e-310, 4.1e-300, -4e-300, 7e-307, 1.0000070031003328e-301,
0 };

#if 1
#define REP1 (1 << 23) /* Repeat factor for second set of one term tests */
#define REP10 (1 << 13) /* Repeat factor for second set of ten term tests */
#else
#define REP1 2 /* Small repetition count maybe sometimes useful for debugging */
#define REP10 2
#endif

int echo, debug_all, debug_letter, debug_number;

int different (double a, double b)
{ 
  return isnan(a) != isnan(b) || !isnan(a) && !isnan(b) && a != b;
}

void small_result (xsum_small_accumulator *sacc, double s, int i)
{
  double r, r2;
  if (xsum_debug) xsum_small_display(sacc);
  r = xsum_small_round (sacc);
  r2 = xsum_small_round (sacc);
  if (xsum_debug) xsum_small_display(sacc);
  if (different(r,r2))
  { printf("%3d small: Different second time %.16le %.16le\n", i, r, r2);
  }
  if (different(r,s))
  { printf("%3d small: Result incorrect %.16le %.16le\n", i, r, s);
    printf("    "); pbinary_double(r); printf("\n");
    printf("    "); pbinary_double(s); printf("\n");
  }
}

void large_result (xsum_large_accumulator *lacc, double s, int i)
{
  double r, r2;
  if (xsum_debug) xsum_large_display(lacc);
  r = xsum_large_round (lacc);
  r2 = xsum_large_round (lacc);
  if (xsum_debug) xsum_large_display(lacc);
  if (different(r,r2))
  { printf("%3d large: Different second time %.16le %.16le\n", i, r, r2);
  }
  if (different(r,s))
  { printf("%3d large: Result incorrect %.16le %.16le\n", i, r, s);
    printf("    "); pbinary_double(r); printf("\n");
    printf("    "); pbinary_double(s); printf("\n");
  }
}

int main (int argc, char **argv)
{
  xsum_small_accumulator sacc, sacc2;
  xsum_large_accumulator lacc, lacc2;
  double s;
  int done;
  int i, j;

  if (argc>2 || argc==2 && strcmp(argv[1],"-e")!=0 
                        && (argv[1][0]!='-' || argv[1][1]!='d'))
  { fprintf(stderr,"Usage: xsum-check [ -e | -d[L#] ]\n");
    exit(1);
  }
 
  echo = argc > 1;
  debug_all = debug_letter = debug_number = 0;
  if (argc > 1 && argv[1][1]=='d')
  { debug_all = argv[1][2]==0;
    if (!debug_all)
    { debug_letter = argv[1][2];
      debug_number = atoi(argv[1]+3);
    }
  }

  /* On an Intel machine, set the 387 FPU to do double rounding, in order
     to get correct IEEE 64-bit floating point.  (Only relevant if SSE2
     instructions not used instead of FPU.)  This will disable higher
     precision for long double, however! */

# ifdef DOUBLE
  { unsigned int mode = 0x27f;
    __asm__ ("fldcw %0" : : "m" (*&mode));
  }
# endif

  printf("\nCORRECTNESS TESTS\n");

  printf("\nA: ZERO TERM TEST\n");

  if (echo) printf(" \n-- TEST 0: \n");
  if (echo) printf("   ANSWER:  %.16le\n",0.0);

  xsum_debug = debug_all || debug_letter=='A';

  xsum_small_init (&sacc);
  small_result(&sacc,0,0);

  xsum_large_init (&lacc);
  large_result(&lacc,0,0);

  printf("\nB: ONE TERM TESTS\n");

  for (i = 0; one_term[i] != 0; i += 1)
  { 
    if (echo) printf(" \n-- TEST %2d: %.16le\n",i,one_term[i]);
    s = one_term[i];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter=='B' && debug_number==i;

    xsum_small_init (&sacc);
    xsum_small_add1 (&sacc, one_term[i]);
    small_result(&sacc,s,i);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, one_term+i, 1);
    large_result(&lacc,s,i);
  }

  printf("\nC: ONE TERM TESTS TIMES %d\n",REP1);

  for (i = 0; one_term[i] != 0; i += 1)
  { 
    if (echo) printf(" \n-- TEST %2d: %.16le\n",i,one_term[i]);
    s = one_term[i] * REP1;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter=='C' && debug_number==i;

    xsum_small_init (&sacc);
    for (j = 0; j < REP1; j++) xsum_small_add1 (&sacc, one_term[i]);
    small_result(&sacc,s,i);

    xsum_large_init (&lacc);
    for (j = 0; j < REP1; j++) xsum_large_addv (&lacc, one_term+i, 1);
    large_result(&lacc,s,i);
  }

  printf("\nD: TWO TERM TESTS\n");

  for (i = 0; two_term[i] != 0; i += 2)
  { 
    if (echo) printf(" \n-- TEST %2d: %.16le %.16le\n",
                      i/2,two_term[i],two_term[i+1]);
    s = two_term[i] + two_term[i+1];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter=='D' && debug_number==i/2;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, two_term+i, 2);
    small_result(&sacc,s,i/2);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, two_term+i, 2);
    large_result(&lacc,s,i/2);
  }

  printf("\nE: THREE TERM TESTS\n");

  for (i = 0; three_term[i] != 0; i += 4)
  { 
    if (echo) printf(" \n-- TEST %2d: %.16le %.16le %.16le\n",
                      i/4,three_term[i],three_term[i+1],three_term[i+2]);
    s = three_term[i+3];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter=='E' && debug_number==i/4;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, three_term+i, 3);
    small_result(&sacc,s,i/4);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, three_term+i, 3);
    large_result(&lacc,s,i/4);
  }

  printf("\nF: TEN TERM TESTS\n");

  for (i = 0; ten_term[i] != 0; i += 11)
  { 
    if (echo) printf(" \n-- TEST %2d\n",i/11);
    s = ten_term[i+10];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter=='F' && debug_number==i/11;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, ten_term+i, 10);
    small_result(&sacc,s,i/11);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, ten_term+i, 10);
    large_result(&lacc,s,i/11);
  }

  printf("\nG: TEN TERM TESTS TIMES %d\n",REP10);

  for (i = 0; ten_term[i] != 0; i += 11)
  { 
    if (echo) printf(" \n-- TEST %2d\n",i/11);
    s = ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter=='G' && debug_number==i/11;

    xsum_small_init (&sacc);
    for (j = 0; j < REP10; j++) xsum_small_addv (&sacc, ten_term+i, 10);
    small_result(&sacc,s,i/11);

    xsum_large_init (&lacc);
    for (j = 0; j < REP10; j++) xsum_large_addv (&lacc, ten_term+i, 10);
    large_result(&lacc,s,i/11);
  }

  printf("\nH: TESTS OF ADDING TOGETHER ACCUMULATORS\n");

  done = 0;
  for (i = 0; !done; i += 1)
  { 
    xsum_debug = debug_all || debug_letter=='H' && debug_number==i;
    if (echo) printf(" \n-- TEST %2d\n",i);
    s = 1234.5;

    xsum_small_init (&sacc);
    xsum_small_init (&sacc2);
    xsum_large_init (&lacc);
    xsum_large_init (&lacc2);

    switch (i)
    { case 0:  /* add one small/large accumulator to another */
      { if (echo) printf("   ANSWER:  %.16le\n",s);
        double v1[3] = { 3.7e20, 888.8, 4.1e20 };
        double v2[4] = { s, -4.1e20, -3.7e20, -888.8 };
        xsum_small_addv (&sacc, v1, 3);
        xsum_small_addv (&sacc2, v2, 4);
        xsum_small_add_accumulator (&sacc, &sacc2);
        xsum_large_addv (&lacc, v1, 3);
        xsum_large_addv (&lacc2, v2, 4);
        xsum_large_add_accumulator (&lacc, &lacc2);
        small_result(&sacc,s,i);
        large_result(&lacc,s,i);
        break;
      }
      case 1:  /* add a small/large accumulator many times */
      { if (echo) printf("   ANSWER:  %.16le\n",s);
        double v1[3] = { 3.5, 888.75, 4.125 };
        double v2[3] = { -4.125, -3.5, -888.75 };
        int k;

        xsum_small_addv (&sacc2, v1, 3);
        for (k = 0; k < 1000; k++) xsum_small_add_accumulator (&sacc, &sacc2);
        xsum_small_add1 (&sacc, s);
        xsum_small_addv (&sacc2, v2, 3);
        xsum_small_addv (&sacc2, v2, 3);
        for (k = 0; k < 1000; k++) xsum_small_add_accumulator (&sacc, &sacc2);

        xsum_large_addv (&lacc2, v1, 3);
        for (k = 0; k < 1000; k++) xsum_large_add_accumulator (&lacc, &lacc2);
        xsum_large_add1 (&lacc, s);
        xsum_large_addv (&lacc2, v2, 3);
        xsum_large_addv (&lacc2, v2, 3);
        for (k = 0; k < 1000; k++) xsum_large_add_accumulator (&lacc, &lacc2);

        small_result(&sacc,s,i);
        large_result(&lacc,s,i);

        break;
      }
      case 2:  /* add a small/large accumulator many times, producing +inf */
      { if (echo) printf("   ANSWER:  +inf\n");

        double v1[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isinf(r) || r < 0) 
        { printf("%d small: Result not +inf: %.16le\n",i,r);
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isinf(r) || r < 0) 
        { printf("%d large: Result not +inf: %.16le\n",i,r);
        }

        break;
      }
      case 3:  /* add a small/large accumulator MANY times, producing NaN */
      { if (echo) printf("   ANSWER:  NaN\n");

        double v1[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isnan(r))
        { printf("%d small: Result not NaN: %.16le\n",i,r);
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isnan(r))
        { printf("%d large: Result not NaN: %.16le\n",i,r);
        }

        break;
      }
      case 4:  /* add a small/large accumulator many times, producing -inf */
      { if (echo) printf("   ANSWER:  -inf\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isinf(r) || r > 0) 
        { printf("%d small: Result not -inf: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isinf(r) || r > 0) 
        { printf("%d large: Result not -inf: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 5:  /* add a small/large accumulator MANY times, producing NaN */
      { if (echo) printf("   ANSWER:  NaN\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isnan(r))
        { printf("%d small: Result not NaN: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isnan(r))
        { printf("%d large: Result not NaN: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 6:  /* add a small/large accumulator many times, producing zero */
      { if (echo) printf("   ANSWER:  0\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double v2[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        xsum_small_addv (&sacc2, v2, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc2;
          xsum_small_add_accumulator (&sacc2, &t);
        }

        xsum_small_add_accumulator (&sacc, &sacc2);

        r = xsum_small_round(&sacc);
        if (isnan(r) || r != 0)
        { printf("%d small: Result not 0: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 7:  /* add a small/large accumulator MANY times, producing NaN */
      { if (echo) printf("   ANSWER:  NaN\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double v2[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        xsum_small_addv (&sacc2, v2, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc2;
          xsum_small_add_accumulator (&sacc2, &t);
        }

        xsum_small_add_accumulator (&sacc, &sacc2);

        r = xsum_small_round(&sacc);
        if (!isnan(r))
        { printf("%d small: Result not NaN: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        done = 1;
      }
    }
  }

  printf("\nI: SPECIAL TESTS\n");

  done = 0;
  for (i = 0; !done; i += 1)
  { 
    xsum_debug = debug_all || debug_letter=='I' && debug_number==i;
    if (echo) printf(" \n-- TEST %2d\n",i);
    s = 1234.5;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_small_init (&sacc);
    xsum_large_init (&lacc);

    switch (i)
    { case 0:  /* add positive zero to 1234.5 */
      { double v[2] = { s, zero };
        xsum_small_addv (&sacc, v, 2);
        xsum_large_addv (&lacc, v, 2);
        break;
      }
      case 1:  /* add negative zero to 1234.5 */
      { double v[2] = { s, -zero };
        xsum_small_addv (&sacc, v, 2);
        xsum_large_addv (&lacc, v, 2);
        break;
      }
      case 2:  /* cause simulataneous moves of pair from large to small */
      { double v[20001];
        int j;
        for (j = 0; j < 10000; j += 2)
        { v[j] = 4567;
          v[j+1] = 10000;
        }
        for (j = 10000; j < 20000; j += 2)
        { v[j] = -4567;
          v[j+1] = -10000;
        }
        v[20000] = s;
        xsum_small_addv (&sacc, v, 20001);
        xsum_large_addv (&lacc, v, 20001);
        break;
      }
      case 3:  /* cause move of pair from large to small same time as init */
      { double v [2 * (1 << XSUM_LCOUNT_BITS) + 3];
        int j;
        for (j = 0; j < (1 << XSUM_LCOUNT_BITS); j += 1)
        { v[j] = 2345;
        }
        v [(1 << XSUM_LCOUNT_BITS)] = 2345;
        v [(1 << XSUM_LCOUNT_BITS) + 1] = s;
        for (j = (1 << XSUM_LCOUNT_BITS) + 2; 
             j < 2 * (1 << XSUM_LCOUNT_BITS) + 3; 
             j += 1)
        { v[j] = -2345;
        }
        xsum_small_addv (&sacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        xsum_large_addv (&lacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        break;
      }
      case 4:  /* as above, but with order of pair involved reversed */
      { double v [2 * (1 << XSUM_LCOUNT_BITS) + 3];
        int j;
        for (j = 0; j < (1 << XSUM_LCOUNT_BITS); j += 1)
        { v[j] = 2345;
        }
        v [(1 << XSUM_LCOUNT_BITS)] = s;
        v [(1 << XSUM_LCOUNT_BITS) + 1] = 2345;
        for (j = (1 << XSUM_LCOUNT_BITS) + 2; 
             j < 2 * (1 << XSUM_LCOUNT_BITS) + 3; 
             j += 1)
        { v[j] = -2345;
        }
        xsum_small_addv (&sacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        xsum_large_addv (&lacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        break;
      }
      case 5:  /* test transfer to small accumulator */
      { double v[3] = { 5100, s, -5100 };
        xsum_large_addv (&lacc, v, 3);
        xsum_large_to_small_accumulator (&sacc, &lacc);
        done = 1;
      }
    }

    small_result(&sacc,s,i);
    large_result(&lacc,s,i);
  }
  
  printf("\nDONE\n\n");
  
  return 0;
}
