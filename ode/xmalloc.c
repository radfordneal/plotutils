/*
 * memory management wrappers for ode.
 * Copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

Voidptr 
xmalloc (size)
     unsigned int size;
{
  Voidptr p;
  p = (Voidptr) malloc (size);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("malloc failed");
      exit (1);
    }
  return p;
}

char *
xstrdup (s)
     const char *s;
{
  char *t = (char *)xmalloc (strlen (s) + 1);

  strcpy (t, s);
  return t;
}
