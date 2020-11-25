/* Wrappers for standard storage allocation functions.  The tests for zero
   size, etc., are necessitated by the calling conventions of the MI scan
   conversion module. */

#include "sys-defines.h"
#include "extern.h"

/* wrapper for malloc() */
Voidptr 
#ifdef _HAVE_PROTOS
mi_xmalloc (unsigned int size)
#else
mi_xmalloc (size)
     unsigned int size;
#endif
{
  Voidptr p;

  if (size == 0)
    return (Voidptr)NULL;

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
mi_xcalloc (unsigned int nmemb, unsigned int size)
#else
mi_xcalloc (nmemb, size)
     unsigned int nmemb, size;
#endif
{
  Voidptr p;

  if (size == 0)
    return (Voidptr)NULL;

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
mi_xrealloc (Voidptr p, unsigned int size)
#else
mi_xrealloc (p, size)
     Voidptr p;
     unsigned int size;
#endif
{
  if (!p)
    return mi_xmalloc (size);
  else
    {
      if (size == 0)
	{
	  free (p);
	  return (Voidptr)NULL;
	}
      
      p = (Voidptr) realloc (p, size);
      if (p == (Voidptr)NULL)
	{
	  fprintf (stderr, "libplot: ");
	  perror ("out of memory");
	  exit (EXIT_FAILURE);
	}
      return p;
    }
}
