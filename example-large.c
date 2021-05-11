/* Example program using 'xsum' routines with large superaccumulator.

   Compile with:

   gcc -O3 example-large.c xsum.c -o example-large
*/

#include <stdio.h>
#include "xsum.h"

#define N 100000000
double a[N];

int main (void)
{ 
  xsum_large_accumulator exact_sum;
  double inexact_sum_f, inexact_sum_b;
  int i;

  xsum_large_init (&exact_sum);
  inexact_sum_f = inexact_sum_b = 0;

  for (i = 1; i <= N; i++)
  { inexact_sum_f += 1.0/i;
    inexact_sum_b += 1.0/(N-i+1);
    a[i-1] = 1.0/i;
  }

  xsum_large_addv (&exact_sum, a, N);

  printf ("Exactly-rounded sum:  %.16f\n", xsum_large_round (&exact_sum));
  printf ("Forward inexact sum:  %.16f\n", inexact_sum_f);
  printf ("Backward inexact sum: %.16f\n", inexact_sum_b);
  
  return 0;
}
