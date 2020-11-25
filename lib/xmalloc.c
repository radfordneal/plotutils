#include "sys-defines.h"

/* forward references */
voidptr_t xmalloc ____P((size_t length));
voidptr_t xrealloc ____P((voidptr_t p, size_t length));
voidptr_t xcalloc ____P((size_t nmemb, size_t size));

voidptr_t 
#ifdef _HAVE_PROTOS
xmalloc (size_t length)
#else
xmalloc (length)
     size_t length;
#endif
{
  voidptr_t p;
  p = (voidptr_t) malloc (length);

  if (p == (voidptr_t) NULL)
    {
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

voidptr_t 
#ifdef _HAVE_PROTOS
xrealloc (voidptr_t p, size_t length)
#else
xrealloc (p, length)
     voidptr_t p;
     size_t length;
#endif
{
  p = (voidptr_t) realloc (p, length);

  if (p == (voidptr_t) NULL)
    {
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

voidptr_t 
#ifdef _HAVE_PROTOS
xcalloc (size_t nmemb, size_t size)
#else
xcalloc (nmemb, size)
     size_t nmemb, size;
#endif
{
  voidptr_t p;
  p = (voidptr_t) calloc (nmemb, size);

  if (p == (voidptr_t) NULL)
    {
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}
