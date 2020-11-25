#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* _matrix_product computes the product of two PS-style transformation
   matrices (i.e. matrix representations of affine transformations). */

void
_matrix_product (m, n, product)
     const double m[6], n[6];
     double product[6];
{
  double local_product[6];

  local_product[0] = m[0] * n[0] + m[1] * n[2];
  local_product[1] = m[0] * n[1] + m[1] * n[3];

  local_product[2] = m[2] * n[0] + m[3] * n[2];  
  local_product[3] = m[2] * n[1] + m[3] * n[3];

  local_product[4] = m[4] * n[0] + m[5] * n[2] + n[4];
  local_product[5] = m[4] * n[1] + m[5] * n[3] + n[5];

  memcpy (product, local_product, 6 * sizeof (double));

  return;
}

/* _matrix_norm computes the matrix norm (in the l^2 sense) of the linear
   transformation part of a PS-style transformation matrix.  Actually we
   compute instead the geometric mean of the l^1 and l^infinity norms.  By
   Hadamard's 3-line theorem, this geometric mean is an upper bound on the
   true l^2 norm.

   This function is called only by common/space.c, libplotps/alabel_ps.c
   and libplotps/point.c, to select appropriate line widths and font sizes.
   For the purposes of those functions, the above approximation should
   suffice. */

double 
_matrix_norm (m)
     const double m[6];
{
  double mt[4], pm[4];
  double norm1, norm2;
  double a[4];
  int i;
  
  mt[0] = m[0];			/* transpose of m */
  mt[1] = m[2];
  mt[2] = m[1];
  mt[3] = m[3];
  
  pm[0] = m[0] * mt[0] + m[1] * mt[2]; /* pm = m * mt */
  pm[1] = m[0] * mt[1] + m[1] * mt[3];  
  pm[2] = m[2] * mt[0] + m[3] * mt[2];
  pm[3] = m[2] * mt[1] + m[3] * mt[3];  

  for (i = 0; i < 4; i++)
    a[i] = fabs(pm[i]);
  
  /* compute l^1 and l^infinity norms of m * mt */
  norm1 = DMAX(a[0]+a[1], a[2]+a[3]);
  norm2 = DMAX(a[0]+a[2], a[1]+a[3]);  
  
 /* l^2 norm of m is sqrt of l^2 norm of m * mt */
  return sqrt(sqrt(norm1 * norm2));
}     
