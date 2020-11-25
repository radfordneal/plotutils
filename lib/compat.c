/* Replacement for system strerror() if it doesn't exist. */

#include "sys-defines.h"

#ifndef HAVE_STRERROR

/* external */
extern char *sys_errlist[];
extern int sys_nerr;

char *
#ifdef _HAVE_PROTOS
strerror (int errnum)
#else
strerror (errnum)
     int errnum;
#endif
{
  if (errnum < 0 || errnum >= sys_nerr)
    return "unknown error";

  return sys_errlist[errnum];
}
#endif /* not HAVE_STRERROR */
