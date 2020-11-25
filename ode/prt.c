#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * print queue memory management
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

struct prt *
#ifdef _HAVE_PROTOS
palloc (void)
#else
palloc ()
#endif
{
  struct prt *pp;
  
  pp = (struct prt *)xmalloc (sizeof(struct prt));
  pp->pr_sym = NULL;
  pp->pr_link = NULL;
  pp->pr_which = P_VALUE;	/* default */
  return pp;
}

void
#ifdef _HAVE_PROTOS
pfree (struct prt *pp)
#else
pfree (pp)
     struct prt *pp;
#endif
{
  if (pp != NULL) 
    {
      pfree (pp->pr_link);
      free ((Voidptr)pp);
    }
}
