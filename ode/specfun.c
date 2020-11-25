/* Special function approximations by Jos van der Woude <jvdwoude@hut.nl>,
   Copyright (c) 1992 by Jos van der Woude.  Permission granted to
   distribute freely for non-commercial purposes only.

   Extracted from the file specfun.c in the gnuplot 3.5 distribution.

   This file contains the functions

   ibeta, igamma, inverf, invnorm, norm

   and versions of erf, erfc, lgamma for machines without them. */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include <errno.h>

#define ITMAX   100
#ifdef FLT_EPSILON
#define MACHEPS FLT_EPSILON /* 1.0E-08 */
#else
#define MACHEPS 1.0E-08
#endif
#ifdef FLT_MIN_EXP
#define MINEXP  FLT_MIN_EXP /* -88.0 */
#else
#define MINEXP  -88.0
#endif
#ifdef FLT_MAX
#define OFLOW   FLT_MAX /* 1.0E+37 */
#else
#define OFLOW   1.0E+37
#endif
#ifdef FLT_MAX_10_EXP
#define XBIG    FLT_MAX_10_EXP /* 2.55E+305 */
#else
#define XBIG    2.55E+305
#endif

#ifndef HUGE_VAL
#ifdef HUGE
#define HUGE_VAL HUGE
#else
#ifdef INF
#define HUGE_VAL INF
#else
#define HUGE_VAL OFLOW
#endif
#endif
#endif

/*
 * Mathematical constants
 */
#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846
#ifdef M_SQRT2
#undef M_SQRT2
#endif
#define M_SQRT2 1.41421356237309504880168872420969809   /* JG */
#define M_LNPI 1.14472988584940016
#define M_LNSQRT2PI 0.9189385332046727

/* Forward references */

/* The following gamma-related nonsense is necessary because (1) some
   vendors have lgamma(), some have gamma(), and some have neither [see
   include/sys-defines.h for further comments], (2) some vendors do not
   declare whichever function they have [e.g. Irix 5.3 requires an
   auxiliary preprocessing symbol to be defined for the declaration in
   math.h to be visible], and (3) some vendors supply broken versions which
   we can't use [e.g. AIX's libm.a gamma support is conspicuously broken],
   so we need to link in a replacement, but we can't use the same name for
   the external symbol `signgam'.  What a mess! -- rsm */
#ifdef NO_SYSTEM_GAMMA
#define SIGNGAM our_signgam
static int SIGNGAM;
double f_lgamma __P((double x));
static double lgamneg __P((double x));
static double lgampos __P((double x));
#else  /* not NO_SYSTEM_GAMMA, we link in vendor code */
#define SIGNGAM signgam
extern int SIGNGAM;
#endif
double f_gamma __P((double x));

#ifndef HAVE_ERF
double erf __P((double x));
double erfc __P((double x));
#endif
double ibeta __P((double a, double b, double x));
double igamma __P((double a, double x));
double inverf __P((double p));
double invnorm __P((double p));
double norm __P((double x));
static double confrac __P((double a, double b, double x));

/* Our gamma function.  F_LGAMMA(), which this calls, computes the log of
   the gamma function, with the sign being returned in SIGNGAM.  F_LGAMMA()
   is defined in include/sys-defines.h.  It may be a vendor-supplied
   lgamma(), a vendor-supplied gamma(), or our own f_lgamma (see below). */
double
#ifdef _HAVE_PROTOS
f_gamma (double x)
#else
f_gamma (x)
     double x;
#endif
{
#ifdef HAVE_MATHERR
  struct exception exc;
#endif

  double y = F_LGAMMA(x);
  
  if (y > 88.0) 
    {
#ifdef HAVE_MATHERR
      exc.name = "lgamma";
      exc.arg1 = x;
      exc.retval = HUGE_VAL;
      exc.type = OVERFLOW;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "lgamma: OVERFLOW error\n");
	  errno = ERANGE;
	}
      return exc.retval;
#else
      errno = ERANGE;
      return HUGE_VAL;
#endif
    }
  else
    return SIGNGAM * exp(y);
}

#ifdef NO_SYSTEM_GAMMA
/**
 * from statlib, Thu Jan 23 15:02:27 EST 1992 ***
 *
 * This file contains two algorithms for the logarithm of the gamma
 * function.  Algorithm AS 245 is the faster (but longer) and gives an
 * accuracy of about 10-12 significant decimal digits except for small
 * regions around z = 1 and z = 2, where the function goes to zero.  
 * 
 * The second algorithm (used below) is not part of the AS algorithms.
 * It is slower but gives 14 or more significant decimal digits accuracy,
 * except around z = 1 and z = 2.  The Lanczos series from which this
 * algorithm is derived is interesting in that it is a convergent series
 * approximation for the gamma function, whereas the familiar series due to
 * De Moivre (and usually wrongly called Stirling's approximation) is only
 * an asymptotic approximation, as is the true and preferable approximation
 * due to Stirling.

 * Uses Lanczos-type approximation to log(gamma(z)) for z > 0. 
 * 
 * Reference: C. Lanczos, `A precision approximation of the 
 * 	      gamma function', SIAM J. Numer. Anal., B, 1, 86-96, 1964.  
 * Accuracy: about 14 significant digits except for small regions in 
 * 	     the vicinity of 1 and 2.
 *
 * Programmer: Alan Miller, CSIRO Division of Mathematics & Statistics
 * 
 * Latest revision: 17 April 1988
 * 
 * Additions: Translated from Fortran to C, code added to handle values z < 0.
 * The global variable SIGNGAM contains the sign of the gamma function.
 * 
 * IMPORTANT: The SIGNGAM variable contains garbage until AFTER the call to
 * lgamma().
 * 
 * Permission granted to distribute freely for non-commercial purposes only
 * Copyright (c) 1992 Jos van der Woude <jvdwoude@hut.nl>
 */

/* high-precision values from Ray Toy <toy@rtp.ericsson.se> */
static double a[] = 
{
       .99999999999980993227684700473478296744476168282198,
    676.52036812188509856700919044401903816411251975244084,
  -1259.13921672240287047156078755282840836424300664868028,
    771.32342877765307884865282588943070775227268469602500,
   -176.61502916214059906584551353999392943274507608117860,
     12.50734327868690481445893685327104972970563021816420,
      -.13857109526572011689554706984971501358032683492780,
       .00000998436957801957085956266828104544089848531228,
       .00000015056327351493115583383579667028994545044040,
};

static double   
#ifdef _HAVE_PROTOS
lgamneg (double z)
#else
lgamneg (z)
     double z;
#endif
{
  double tmp, result;
#ifdef HAVE_MATHERR
  struct exception exc;
#endif

  if (!modf (z = -z, &tmp)) 
    /* SING if z is negative integer */
    {
#ifdef HAVE_MATHERR
      exc.name = "lgamma";
      exc.arg1 = -z;
      exc.retval = HUGE_VAL;
      exc.type = SING;
      if (!matherr (&exc)) 
	{
	  fprintf (stderr, "lgamma: SING error\n");
	  errno = EDOM;
	}
      return (exc.retval);
#else
      errno = EDOM;
      return HUGE_VAL;
#endif /* HAVE_MATHERR */
    }

  /* Use reflection formula, then call lgampos() */
  tmp = sin(z * M_PI);
  
  if (tmp < 0.0) 
    {
      tmp = -tmp;
      SIGNGAM = -1;
    }
  result = M_LNPI - lgampos(1.0 - z) - log(tmp);

  if (fabs (result) == HUGE_VAL)
    {
#ifdef HAVE_MATHERR
      exc.name = "gamma";
      exc.arg1 = z;
      exc.retval = HUGE_VAL;
      exc.type = OVERFLOW;
      if (!matherr(&exc))
	errno = ERANGE;
      return (exc.retval);
#else
      errno = ERANGE;
      return HUGE_VAL;
#endif
    }

  return result;
}

static double   
#ifdef _HAVE_PROTOS
lgampos (double z)
#else
lgampos (z)
     double z;
#endif
{
  double          sum;
  double          tmp;
  double          result;
  int             i;
#ifdef HAVE_MATHERR
  struct exception exc;
#endif
  
  sum = a[0];
  for (i = 1, tmp = z; i < 9; i++) 
    {
      sum += a[i] / tmp;
      tmp++;
    }
  result = log(sum) + M_LNSQRT2PI - z - 6.5 + (z - 0.5) * log(z + 6.5);

  if (result == HUGE_VAL)
    {
#ifdef HAVE_MATHERR
      exc.name = "lgamma";
      exc.arg1 = z;
      exc.retval = HUGE_VAL;
      exc.type = OVERFLOW;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "lgamma: OVERFLOW error\n");
	  errno = ERANGE;
	}
      return exc.retval;
#else
      errno = ERANGE;
      return HUGE_VAL;
#endif
    }

  return result;
}

/* Our log-of-gamma function, which we use if the vendor doesn't supply
   one, or if the vendor's version is buggy. */
double 
#ifdef _HAVE_PROTOS
f_lgamma (double z)
#else
f_lgamma (z)
     double z;
#endif
{
  SIGNGAM = 1;
  
  if (z <= 0.0)
    return lgamneg(z);
  else
    return lgampos(z);
}
#endif /* NO_SYSTEM_GAMMA */

#ifndef HAVE_ERF
double
#ifdef _HAVE_PROTOS
erf (double x)
#else
erf (x)
     double x;
#endif
{
  x = x < 0.0 ? -igamma(0.5, x * x) : igamma(0.5, x * x);

  return x;
}

double
#ifdef _HAVE_PROTOS
erfc (double x)
#else
erfc (x)
     double x;
#endif
{
  x = x < 0.0 ? 1.0 + igamma(0.5, x * x) : 1.0 - igamma(0.5, x * x);

  return x;
}
#endif /* not HAVE_ERF */

/** ibeta.c
 *
 *   DESCRIB   Approximate the incomplete beta function Ix(a, b).
 *
 *                           
 *                     gamma(a + b)      /x  (a-1)         (b-1)
 *      Ix(a, b) = ------------------- * |  t     * (1 - t)     dt (a,b > 0)
 *                 gamma(a) * gamma(b)   /0
 *
 *
 *
 *   CALL      p = ibeta(a, b, x)
 *
 *             double    a    > 0
 *             double    b    > 0
 *             double    x    [0, 1]
 *
 *   WARNING   none
 *
 *   RETURN    double    p    [0, 1]
 *                            -1.0 on error condition
 *
 *   XREF      lgamma()
 *
 *   BUGS      none
 *
 *   REFERENCE The continued fraction expansion as given by
 *             Abramowitz and Stegun (1964) is used.
 *
 * Permission granted to distribute freely for non-commercial purposes only
 * Copyright (c) 1992 Jos van der Woude <jvdwoude@hut.nl>
 */

double
#ifdef _HAVE_PROTOS
ibeta (double a, double b, double x)
#else
ibeta (a, b, x)
     double a, b, x;
#endif
{
  /* Test for admissibility of arguments */
  if (a <= 0.0 || b <= 0.0 || x < 0.0 || x > 1.0)
    return -1.0;		/* DOMAIN error */
  
  /* If x equals 0 or 1, return x as prob */
  if (x == 0.0 || x == 1.0)
    return x;
  
  /* Swap a, b if necessarry for more efficient evaluation */
  return a < x * (a + b) ? 1.0 - confrac(b, a, 1.0 - x) : confrac(a, b, x);
}

static double   
#ifdef _HAVE_PROTOS
confrac (double a, double b, double x)
#else
confrac (a, b, x)
     double a, b, x;
#endif
{
  double          Alo = 0.0;
  double          Ahi;
  double          Aev;
  double          Aod;
  double          Blo = 1.0;
  double          Bhi = 1.0;
  double          Bod = 1.0;
  double          Bev = 1.0;
  double          f;
  double          fold;
  double          Apb = a + b;
  double          d;
  int             i;
  int             j;

  /* Set up continued fraction expansion evaluation. */
  Ahi = exp(F_LGAMMA(Apb) + a * log(x) + b * log(1.0 - x) -
	    F_LGAMMA(a + 1.0) - F_LGAMMA(b));

  /*
   * Continued fraction loop begins here. Evaluation continues until
   * maximum iterations are exceeded, or convergence achieved.
   */
  for (i = 0, j = 1, f = Ahi; i <= ITMAX; i++, j++) 
    {
      d = a + j + i;
      Aev = -(a + i) * (Apb + i) * x / d / (d - 1.0);
      Aod = j * (b - j) * x / d / (d + 1.0);
      Alo = Bev * Ahi + Aev * Alo;
      Blo = Bev * Bhi + Aev * Blo;
      Ahi = Bod * Alo + Aod * Ahi;
      Bhi = Bod * Blo + Aod * Bhi;
      
      if (fabs (Bhi) < MACHEPS)
	    Bhi = 0.0;
      
      if (Bhi != 0.0) 
	{
	  fold = f;
	  f = Ahi / Bhi;
	  if (fabs(f - fold) < fabs(f) * MACHEPS)
	    return f;
	}
    }
  
  return -1.0;
}

/* igamma.c
 *
 *   DESCRIB   Approximate the incomplete gamma function P(a, x).
 *
 *                           1      /x  -t   (a-1)
 *             P(a, x) = -------- * |  e  * t     dt      (a > 0)
 *                       gamma(a)   /0
 *
 *   CALL      p = igamma(a, x)
 *
 *             double    a    >  0
 *             double    x    >= 0
 *
 *   WARNING   none
 *
 *   RETURN    double    p    [0, 1]
 *                            -1.0 on error condition
 *
 *   XREF      lgamma()
 *
 *   BUGS      Values 0 <= x <= 1 may lead to inaccurate results.
 *
 *   REFERENCE ALGORITHM AS239  APPL. STATIST. (1988) VOL. 37, NO. 3
 *
 * Permission granted to distribute freely for non-commercial purposes only
 * Copyright (c) 1992 Jos van der Woude <jvdwoude@hut.nl>
 */

double
#ifdef _HAVE_PROTOS
igamma (double a, double x)
#else
igamma (a, x)
     double a, x;
#endif
{
  double          arg;
  double          aa;
  double          an;
  double          b;
  double          pn1, pn2, pn3, pn4, pn5, pn6;
  int             i;
  
  /* Check that we have valid values for a and x */
  if (x < 0.0 || a <= 0.0)
    return -1.0;		/* DOMAIN error */
  
  /* Deal with special cases */
  if (x == 0.0)
    return 0.0;			/* SING error */
  if (x > XBIG)
    return 1.0;			/* loss of significance? */
  
  /* Check value of factor arg */
  arg = a * log(x) - x - F_LGAMMA(a + 1.0);
  if (arg < MINEXP)
    return -1.0;
  arg = exp(arg);
  
  /* Choose infinite series or continued fraction. */
  
  if ((x > 1.0) && (x >= a + 2.0)) 
    /* Use a continued fraction expansion */
    {
      double          rn, rnold;
      
      aa = 1.0 - a;
      b = aa + x + 1.0;
      pn1 = 1.0;
      pn2 = x;
      pn3 = x + 1.0;
      pn4 = x * b;
      rnold = pn3 / pn4;
      
      for (i = 1; i <= ITMAX; i++) 
	{
	  aa++;
	  b += 2.0;
	  an = aa * (double) i;
	  
	  pn5 = b * pn3 - an * pn1;
	  pn6 = b * pn4 - an * pn2;
	  
	  if (pn6 != 0.0) 
	    {
	      rn = pn5 / pn6;
	      if (fabs(rnold - rn) <= DMIN(MACHEPS, MACHEPS * rn))
		return 1.0 - arg * rn * a;
	      rnold = rn;
	    }
	  pn1 = pn3;
	  pn2 = pn4;
	  pn3 = pn5;
	  pn4 = pn6;
	  
	  if (fabs(pn5) >= OFLOW) 
	    /* re-scale terms in continued fraction since terms are large */
	    {
	      pn1 /= OFLOW;
	      pn2 /= OFLOW;
	      pn3 /= OFLOW;
	      pn4 /= OFLOW;
	    }
	}
    } 
  else				/* x < max (1, a+2) */
    {
      /* Use Pearson's series expansion. */
      for (i = 0, aa = a, an = b = 1.0; i <= ITMAX; i++) 
	{
	  aa++;
	  an *= x / aa;
	  b += an;
	  if (an < b * MACHEPS)
	    return arg * b;
	}
    }
  return -1.0;
}

/* ----------------------------------------------------------------
   Following additions to specfun.c made by John Grosh <jgrosh@arl.mil>
   on 28 OCT 1992.
   ---------------------------------------------------------------- */

double
#ifdef _HAVE_PROTOS
norm (double x)			/* Normal or Gaussian Probability Function */
#else
norm (x)			/* Normal or Gaussian Probability Function */
     double x;
#endif
{
  /* Ref.: Abramowitz and Stegun 1964, "Handbook of Mathematical
     Functions", Applied Mathematics Series, vol. 55, Chapter 26, page 934,
     Eqn. 26.2.29 and Jos van der Woude code found above */
  
#ifndef ERF
  x = 0.5 * M_SQRT2 * x;
  x = 0.5 * (1.0 + (x < 0.0 ? -igamma(0.5, x * x) : igamma(0.5, x * x)));
#else
  x = 0.5 * (1.0 + erf(0.5 * M_SQRT2 * x));
#endif
  
  return x;
}

double 
#ifdef _HAVE_PROTOS
invnorm (double p)		/* Inverse Normal Probability Function */
#else
invnorm (p)			/* Inverse Normal Probability Function */
     double p;
#endif
{
  /* Source: This routine was derived (using f2c) from the Fortran
   * subroutine MDNRIS found in ACM Algorithm 602, obtained from netlib.
   *
   * MDNRIS code is copyright 1978 by IMSL, Inc.  Since MDNRIS has been
   * submitted to netlib it may be used with the restrictions that it may
   * only be used for noncommercial purposes, and that IMSL be acknowledged
   * as the copyright-holder of the code.
   */

  /* Initialized data */
  static double eps = 1e-10;
  static double g0 = 1.851159e-4;
  static double g1 = -.002028152;
  static double g2 = -.1498384;
  static double g3 = .01078639;
  static double h0 = .09952975;
  static double h1 = .5211733;
  static double h2 = -.06888301;
  
  /* Local variables */
  static double a, w, x;
  static double sd, wi, sn, y;
  
  /* Note: 0.0 < p < 1.0 */
  
  if (p <= eps) 
    /* p too small, compute y directly */
    {
      a = p + p;
      w = sqrt(-(double)log(a + (a - a * a)));
      
      /* use a rational function in 1.0 / w */
      wi = 1.0 / w;
      sn = ((g3 * wi + g2) * wi + g1) * wi;
      sd = ((wi + h2) * wi + h1) * wi + h0;
      y = w + w * (g0 + sn / sd);
      y = - y * M_SQRT2;
    } 
  else 
    {
      x = 1.0 - (p + p);
      y = inverf(x);
      y = - M_SQRT2 * y;
    }
  return y;
} 

double 
#ifdef _HAVE_PROTOS
inverf (double p)		/* Inverse Error Function */
#else
inverf (p)			/* Inverse Error Function */
     double p;
#endif
{
  /* 
   * Source: This routine was derived (using f2c) from the Fortran
   * subroutine MERFI found in ACM Algorithm 602, obtained from netlib.
   * 
   * MDNRIS code is copyright 1978 by IMSL, Inc.  Since MERFI has been
   * submitted to netlib, it may be used with the restrictions that it may
   * only be used for noncommercial purposes, and that IMSL be acknowledged
   * as the copyright-holder of the code.
   */

  /* Initialized data */
  static double a1 = -.5751703;
  static double a2 = -1.896513;
  static double a3 = -.05496261;
  static double b0 = -.113773;
  static double b1 = -3.293474;
  static double b2 = -2.374996;
  static double b3 = -1.187515;
  static double c0 = -.1146666;
  static double c1 = -.1314774;
  static double c2 = -.2368201;
  static double c3 = .05073975;
  static double d0 = -44.27977;
  static double d1 = 21.98546;
  static double d2 = -7.586103;
  static double e0 = -.05668422;
  static double e1 = .3937021;
  static double e2 = -.3166501;
  static double e3 = .06208963;
  static double f0 = -6.266786;
  static double f1 = 4.666263;
  static double f2 = -2.962883;
  static double g0 = 1.851159e-4;
  static double g1 = -.002028152;
  static double g2 = -.1498384;
  static double g3 = .01078639;
  static double h0 = .09952975;
  static double h1 = .5211733;
  static double h2 = -.06888301;

  /* Local variables */
  static double a, b, f, w, x, y, z, sigma, z2, sd, wi, sn;
  
  x = p;
  
  /* determine sign of x */
  sigma = (x > 0 ? 1.0 : -1.0);
  
  /* Note: -1.0 < x < 1.0 */
  
  z = fabs(x);
  
  /* z between 0.0 and 0.85, approx. f by a 
     rational function in z  */
  
  if (z <= 0.85) 
    {
      z2 = z * z;
      f = z + z * (b0 + a1 * z2 / (b1 + z2 + a2 
				   / (b2 + z2 + a3 / (b3 + z2))));
    } 
  else	/* z greater than 0.85 */
    {
    a = 1.0 - z;
    b = z;
    
    /* reduced argument is in (0.85,1.0), obtain the transformed variable */
    
    w = sqrt(-(double)log(a + a * b));
    
    if (w >= 4.0) 
      /* w greater than 4.0, approx. f by a rational function in 1.0 / w */
      {
	wi = 1.0 / w;
	sn = ((g3 * wi + g2) * wi + g1) * wi;
	sd = ((wi + h2) * wi + h1) * wi + h0;
	f = w + w * (g0 + sn / sd);
	
	
    } 
    else if (w < 4.0 && w > 2.5) 
      /* w between 2.5 and 4.0, approx.  f by a rational function in w */
      {
	sn = ((e3 * w + e2) * w + e1) * w;
	sd = ((w + f2) * w + f1) * w + f0;
	f = w + w * (e0 + sn / sd);
	
	/* w between 1.13222 and 2.5, approx. f by 
	   a rational function in w */
      } 
    else if (w <= 2.5 && w > 1.13222) 
      {
	sn = ((c3 * w + c2) * w + c1) * w;
	sd = ((w + d2) * w + d1) * w + d0;
	f = w + w * (c0 + sn / sd);
      }
  }
  y = sigma * f;

  return y;
}
