/* PRINT INTEGERS AND FLOATS IN BINARY. */

/* Copyright 2015, 2018 Radford M. Neal

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

#include "pbinary.h"
#include <stdio.h>


/* PRINT 64-BIT INTEGER IN BINARY.  Prints only the low-order n bits. */

void pbinary_int64 (int64_t v, int n)
{
  int i;
  for (i = n-1; i >= 0; i--)
  { printf ("%d", (int) ((v>>i)&1));
  }
}


/* PRINT DOUBLE-PRECISION FLOATING POINT VALUE IN BINARY. */

void pbinary_double (double d)
{
  union { double f; int64_t i; } u;
  int64_t exp;

  u.f = d;
  printf (u.i < 0 ? "- " : "+ ");
  exp = (u.i >> 52) & 0x7ff;
  pbinary_int64 (exp, 11);
  if (exp == 0) printf (" (denorm) ");
  else if (exp == 0x7ff) printf (" (InfNaN) ");
  else printf(" (%+06d) ", (int) (exp - 1023));
  pbinary_int64 (u.i & 0xfffffffffffffL, 52);
}
