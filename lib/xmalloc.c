#include "sys-defines.h"

/* forward references */
Voidptr xmalloc ____P((unsigned int length));
Voidptr xrealloc ____P((Voidptr p, unsigned int length));
Voidptr xcalloc ____P((unsigned int nmemb, unsigned int size));

Voidptr 
#ifdef _HAVE_PROTOS
xmalloc (unsigned int length)
#else
xmalloc (length)
     unsigned int length;
#endif
{
  Voidptr p;
  p = (Voidptr) malloc (length);

  if (p == (Voidptr) NULL)
    {
      perror ("malloc failed");
      exit (EXIT_FAILURE);
    }
  return p;
}

Voidptr 
#ifdef _HAVE_PROTOS
xrealloc (Voidptr p, unsigned int length)
#else
xrealloc (p, length)
     Voidptr p;
     unsigned int length;
#endif
{
  p = (Voidptr) realloc (p, length);

  if (p == (Voidptr) NULL)
    {
      perror ("realloc failed");
      exit (EXIT_FAILURE);
    }
  return p;
}

Voidptr 
#ifdef _HAVE_PROTOS
xcalloc (unsigned int nmemb, unsigned int size)
#else
xcalloc (nmemb, size)
     unsigned int nmemb, size;
#endif
{
  Voidptr p;
  p = (Voidptr) calloc (nmemb, size);

  if (p == (Voidptr) NULL)
    {
      perror ("calloc failed");
      exit (EXIT_FAILURE);
    }
  return p;
}
