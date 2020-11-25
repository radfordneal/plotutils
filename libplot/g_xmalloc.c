/* Wrappers for standard storage allocation functions, for
   libplot/libplotter with the exception of the MI scan conversion module,
   which has its own more complicated versions (see g_mialloc.c). */

#include "sys-defines.h"
#include "extern.h"

/* wrapper for malloc() */
Voidptr 
#ifdef _HAVE_PROTOS
_plot_xmalloc (unsigned int size)
#else
_plot_xmalloc (size)
     unsigned int size;
#endif
{
  Voidptr p;

  p = (Voidptr) malloc (size);
  if (p == (Voidptr)NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for calloc() */
Voidptr 
#ifdef _HAVE_PROTOS
_plot_xcalloc (unsigned int nmemb, unsigned int size)
#else
_plot_xcalloc (nmemb, size)
     unsigned int nmemb, size;
#endif
{
  Voidptr p;

  p = (Voidptr) calloc (nmemb, size);
  if (p == (Voidptr)NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for realloc() */
Voidptr 
#ifdef _HAVE_PROTOS
_plot_xrealloc (Voidptr p, unsigned int size)
#else
_plot_xrealloc (p, size)
     Voidptr p;
     unsigned int size;
#endif
{
  p = (Voidptr) realloc (p, size);
  if (p == (Voidptr)NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}
