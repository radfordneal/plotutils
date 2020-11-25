/*
 * lex wrapper for ode.
 * Copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

int
yywrap()
{
  if (yyin != stdin)		/* we were reading from a file, */
    {				/* so switch to reading from stdin */
      yyin = stdin;
      filename = "";
      curline = 1;
      return 0;
    }
  else				/* we were reading from stdin, */
    return 1;			/* so there's no more input */
}
