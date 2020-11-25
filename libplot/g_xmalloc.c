/* Wrappers for standard storage allocation functions, for
   libplot/libplotter with the exception of the libxmi scan conversion
   module, which has its own more complicated versions (see mi_alloc.c). */

#include "sys-defines.h"
#include "extern.h"

/* wrapper for malloc() */
voidptr_t 
#ifdef _HAVE_PROTOS
_plot_xmalloc (size_t size)
#else
_plot_xmalloc (size)
     size_t size;
#endif
{
  voidptr_t p;

  p = (voidptr_t) malloc (size);
  if (p == (voidptr_t)NULL)
    {
      fputs ("libplot: ", stderr);
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for calloc() */
voidptr_t 
#ifdef _HAVE_PROTOS
_plot_xcalloc (size_t nmemb, size_t size)
#else
_plot_xcalloc (nmemb, size)
     size_t nmemb, size;
#endif
{
  voidptr_t p;

  p = (voidptr_t) calloc (nmemb, size);
  if (p == (voidptr_t)NULL)
    {
      fputs ("libplot: ", stderr);
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for realloc() */
voidptr_t 
#ifdef _HAVE_PROTOS
_plot_xrealloc (voidptr_t p, size_t size)
#else
_plot_xrealloc (p, size)
     voidptr_t p;
     size_t size;
#endif
{
  p = (voidptr_t) realloc (p, size);
  if (p == (voidptr_t)NULL)
    {
      fputs ("libplot: ", stderr);
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}
