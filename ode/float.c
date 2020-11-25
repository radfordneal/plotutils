#include "sys-defines.h"
#include <signal.h>
#include "ode.h"
#include "extern.h"

/*
 * arithmetic exceptions (e.g., floating point errors) come here
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

RETSIGTYPE
#ifdef _HAVE_PROTOS
fptrap (int sig)
#else
fptrap (sig)
     int sig;
#endif
{
  rterror ("arithmetic exception");
}

void
#ifdef _HAVE_PROTOS
setflt (void)
#else
setflt ()
#endif
{
  if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
    signal (SIGFPE, fptrap);
}

void
#ifdef _HAVE_PROTOS
resetflt (void)
#else
resetflt ()
#endif
{
  if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
    signal (SIGFPE, SIG_DFL);
}
