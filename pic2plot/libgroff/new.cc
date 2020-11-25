/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "posix.h"

extern const char *program_name;

static void 
ewrite(const char *s)
{
  write(2, s, strlen(s));
}

void *
operator new(size_t size)
{
  // Avoid relying on the behaviour of malloc(0).
  if (size == 0)
    size++;
#ifdef COOKIE_BUG
  char *p = (char *)malloc(unsigned(size + 8));
#else /* not COOKIE_BUG */
  char *p = (char *)malloc(unsigned(size));
#endif /* not COOKIE_BUG */
  if (p == 0) 
    {
      if (program_name) 
	{
	  ewrite(program_name);
	  ewrite(": ");
	}
      ewrite("out of memory\n");
      _exit(-1);
    }
#ifdef COOKIE_BUG
  ((unsigned *)p)[1] = 0;
  return p + 8;
#else /* not COOKIE_BUG */
  return p;
#endif /* not COOKIE_BUG */
}

#ifdef COOKIE_BUG

void 
operator delete(void *p)
{
  if (p)
    free((void *)((char *)p - 8));
}

#endif /* COOKIE_BUG */
