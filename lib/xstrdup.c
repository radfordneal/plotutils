#include "sys-defines.h"

/* forward references */
char * xstrdup ____P((const char *s));

/* external */
extern Voidptr xmalloc ____P((unsigned int length));

char *
#ifdef _HAVE_PROTOS
xstrdup (const char *s)
#else
xstrdup (s)
     const char *s;
#endif
{
  char *t = (char *)xmalloc (strlen (s) + 1);

  strcpy (t, s);
  return t;
}
