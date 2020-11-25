#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

Voidptr 
_plot_xmalloc (size)
     unsigned int size;
{
  Voidptr p;
  p = (Voidptr) malloc (size);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("malloc failed");
      exit (1);
    }
  return p;
}

Voidptr 
_plot_xcalloc (nmemb, size)
     unsigned int nmemb, size;
{
  Voidptr p;
  p = (Voidptr) calloc (nmemb, size);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("calloc failed");
      exit (1);
    }
  return p;
}

Voidptr 
_plot_xrealloc (p, size)
     Voidptr p;
     unsigned int size;
{
  p = (Voidptr) realloc (p, size);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "libplot: ");
      perror ("realloc failed");
      exit (1);
    }
  return p;
}
