/* Example program using routines by Zhu & Hayes.

   Compile with: 

   g++ -O3 example-zhu.cpp ExactSum.cpp -DDOUBLE -o example-zhu
*/

#include <stdio.h>
#include "ExactSum.h"

#define N 100000000
double a[N+1];

int main (void)
{ 
  ExactSum exact_sum;
  double inexact_sum_f, inexact_sum_b;
  int i;

  inexact_sum_f = inexact_sum_b = 0;

  for (i = 1; i <= N; i++)
  { inexact_sum_f += 1.0/i;
    inexact_sum_b += 1.0/(N-i+1);
    a[i] = 1.0/i;
  }

  printf ("Exactly-rounded sum:  %.16f\n", exact_sum.iFastSum(a,N));
  printf ("Forward inexact sum:  %.16f\n", inexact_sum_f);
  printf ("Backward inexact sum: %.16f\n", inexact_sum_b);
  
  return 0;
}
