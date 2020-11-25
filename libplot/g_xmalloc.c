#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

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

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("malloc failed");
      exit (EXIT_FAILURE);
    }
  return p;
}

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

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("calloc failed");
      exit (EXIT_FAILURE);
    }
  return p;
}

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

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("realloc failed");
      exit (EXIT_FAILURE);
    }
  return p;
}
