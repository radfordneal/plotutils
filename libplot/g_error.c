/* This file contains the generic warning and error methods.  They simply
   write the specified message to the plotter error stream, if it has
   one. */

#include "sys-defines.h"
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
    (*libplot_warning_handler)(msg);
  else if (_plotter->errfp)
    fprintf (_plotter->errfp, "libplot: %s\n", msg);
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    (*(_plotter->outstream)) << "libplot: " << msg << '\n';
#endif
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
    (*libplot_error_handler)(msg);
  else if (_plotter->errfp)
    fprintf (_plotter->errfp, "libplot: error: %s\n", msg);
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    (*(_plotter->outstream)) << "libplot: error: " << msg << '\n';
#endif
}
