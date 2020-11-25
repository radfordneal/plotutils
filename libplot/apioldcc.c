/* This file belongs to both libplot and libplotter.  It contains a
   function that appears in both the old (non-thread-safe) C and C++
   bindings.  It is named pl_parampl() and parampl(), respectively.

   pl_parampl/parampl sets parameters in a global PlotterParams object,
   which is used as a source of parameters when any Plotter is created.
   The presence of this global state is one reason why the old API's are
   not thread-safe.

   In libplotter, parampl is a static function member of the Plotter class,
   as is the global PlotterParams.  This is arranged by #ifdef's in
   extern.h.

   In both libplot and libplotter, the pointer to the global PlotterParams,
   which is called _old_api_global_plotter_params, is defined in
   g_defplot.c. */

#include "sys-defines.h"
#include "extern.h"
#ifndef LIBPLOTTER
#include "plot.h"		/* header file for C API's */
#endif

int
#ifdef _HAVE_PROTOS
#ifdef NO_VOID_SUPPORT
#ifdef LIBPLOTTER
parampl (const char *parameter, char *value)
#else  /* not LIBPLOTTER */
pl_parampl (const char *parameter, char *value)
#endif /* not LIBPLOTTER */
#else
#ifdef LIBPLOTTER
parampl (const char *parameter, void *value)
#else  /* not LIBPLOTTER */
pl_parampl (const char *parameter, void *value)
#endif /* not LIBPLOTTER */
#endif
#else  /* not _HAVE_PROTOS, possible only if this is libplot, not libplotter */
#ifdef NO_VOID_SUPPORT
pl_parampl (parameter, value)
     const char *parameter;
     char *value;
#else
pl_parampl (parameter, value)
     const char *parameter;
     void *value;
#endif /* not NO_VOID_SUPPORT */
#endif
{
  /* create global object if necessary (via different routes for libplotter
     and libplot; for latter, call a function in new C API) */
  if (_old_api_global_plotter_params == NULL)
#ifdef LIBPLOTTER
    _old_api_global_plotter_params = new PlotterParams;
#else
    _old_api_global_plotter_params = pl_newplparams ();
#endif

  return _old_api_global_plotter_params->setplparam (R___(_old_api_global_plotter_params)
						     parameter, value);
}

