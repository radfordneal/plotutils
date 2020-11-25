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
fptrap (sig)
     int sig;
{
  rterror ("arithmetic exception");
}

void
setflt ()
{
  if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
    signal (SIGFPE, fptrap);
}

void
resetflt ()
{
  if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
    signal (SIGFPE, SIG_DFL);
}
