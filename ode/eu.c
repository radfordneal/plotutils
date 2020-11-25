#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * Euler's method: quick and dirty.
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

void
eu ()
{
  double t;
  
  for (it = 0, t = tstart; !STOPR; t = tstart + (++it) * tstep) 
    {
      symtab->sy_value = symtab->sy_val[0] = t;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_val[0] = fsp->sy_value;
	  fsp->sy_pri[0] = fsp->sy_prime;
	}
      printq();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)
	fsp->sy_value = fsp->sy_val[0] + tstep * fsp->sy_prime;
    }
}

/*
 * if tstart == tstop, i.e., step nowhere, then just print current values
 */
void
trivial ()
{
  symtab->sy_value = symtab->sy_val[0] = tstart;
  field();
  for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
    {
      fsp->sy_val[0] = fsp->sy_value;
      fsp->sy_pri[0] = fsp->sy_prime;
    }
  printq();
}
