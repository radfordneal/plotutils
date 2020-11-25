#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * find maximum errors
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1999 Free Software Foundation, Inc.
 */

static double   ssemax, abemax, acemax;
static char     *ssenam, *abenam, *acenam;

void
#ifdef _HAVE_PROTOS
maxerr (void)
#else
maxerr ()
#endif
{
  struct sym *sp, *dq;
  
  dq = symtab->sy_link;
  ssemax = abemax = acemax = 0.0;
  for (sp = dq; sp != NULL; sp = sp->sy_link) 
    {
      if (ssemax < sp->sy_sserr) 
	{
	  ssemax = sp->sy_sserr;
	  ssenam = sp->sy_name;
	}
      if (abemax < sp->sy_aberr) 
	{
	  abemax = sp->sy_aberr;
	  abenam = sp->sy_name;
	}
      if (acmax < sp->sy_acerr) 
	{
	  acemax = sp->sy_acerr;
	  acenam = sp->sy_name;
	}
    }
}

bool
#ifdef _HAVE_PROTOS
hierror (void) /* not enough accuracy */
#else
hierror () /* not enough accuracy */
#endif
{
  double t = symtab->sy_val[0];

  if (t + tstep == t) 
    {
      fprintf (stderr, "%s: %s\n", progname, "step size below lower limit");
      longjmp (mark, 1);
    }
  if (ssemax <= ssmax && abemax <= abmax && acemax <= acmax)
    return false;
  if (fabs(tstep) >= fabs(hmin))
    return true;
  if (sflag)
    return false;
  if (ssemax > ssmax)
    fprintf (stderr, 
	     "%s: relative error limit exceeded while calculating %.*s'\n",
	     progname, NAMMAX, ssenam);
  else if (abemax > abmax)
    fprintf (stderr, 
	     "%s: absolute error limit exceeded while calculating %.*s'\n",
	     progname, NAMMAX, abenam);
  else if (acemax > acmax)
    fprintf (stderr, 
	     "%s: accumulated error limit exceeded while calculating %.*s'\n",
	     progname, NAMMAX, acenam);
  longjmp (mark, 1);

  /* doesn't return, but must keep unintelligent compilers happy */
  return false;
}

bool
#ifdef _HAVE_PROTOS
lowerror (void) /* more than enough accuracy */
#else
lowerror () /* more than enough accuracy */
#endif
{
  if (ssemax < ssmin || abemax < abmin)
    if (fabs(tstep) <= fabs(hmax))
      return true;
  return false;
}

/*
 * interpolate to tstop in Runge-Kutta routines
 */
#define PASTSTOP(stepvar) (t + 0.9375*stepvar > tstop && \
                                t + 0.0625*stepvar < tstop)
#define BEFORESTOP(stepvar) (t + 0.9375*stepvar < tstop && \
                                t + 0.0625*stepvar > tstop)

bool
#ifdef _HAVE_PROTOS
intpr (double t)
#else
intpr (t)
     double t;
#endif
{
  if (tstep > 0)
    if (!PASTSTOP(tstep))
      return false;
  if (tstep < 0)
    if (!BEFORESTOP(tstep))
      return false;
  if (tstep > 0)
    while (PASTSTOP(tstep))
      tstep = HALF * tstep;
  if (tstep < 0)
    while (BEFORESTOP(tstep))
      tstep = HALF * tstep;
  return true;
}
