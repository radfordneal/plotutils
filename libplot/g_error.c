/* This file contains the generic error and warning methods.  They simply
   write the specified message to the plotter error stream, if it has one.
   In the case of an error, the program exits. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_g_warning (const char *msg)
#else
_g_warning (msg)
     const char *msg;
#endif
{
  if (libplot_warning_handler != NULL)
    (*libplot_warning_handler)((char *)msg);
  else if (_plotter->errstream != NULL)
    fprintf (_plotter->errstream, "libplot: %s\n", msg);
}

void
#ifdef _HAVE_PROTOS
_g_error (const char *msg)
#else
_g_error (msg)
     const char *msg;
#endif
{
  if (libplot_error_handler != NULL)
    (*libplot_error_handler)((char *)msg);
  else if (_plotter->errstream != NULL)
    fprintf (_plotter->errstream, "libplot: error: %s\n", msg);

  exit (1);
}
