/* Example program using 'xsum' routines with small superaccumulator.

   Compile with:

   gcc -O3 example-small.c xsum.c -o example-small
*/

#include <stdio.h>
#include "xsum.h"

#define N 100000000
double a[N];

int main (void)
{ 
  xsum_small_accumulator exact_sum;
  double inexact_sum_f, inexact_sum_b;
  int i;

  xsum_small_init (&exact_sum);
  inexact_sum_f = inexact_sum_b = 0;

  for (i = 1; i <= N; i++)
  { inexact_sum_f += 1.0/i;
    inexact_sum_b += 1.0/(N-i+1);
    a[i-1] = 1.0/i;
  }

  xsum_small_addv (&exact_sum, a, N);

  printf ("Exactly-rounded sum:  %.16f\n", xsum_small_round (&exact_sum));
  printf ("Forward inexact sum:  %.16f\n", inexact_sum_f);
  printf ("Backward inexact sum: %.16f\n", inexact_sum_b);
  
  return 0;
}
