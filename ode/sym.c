#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * symbol table space management
 * routines
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

struct sym *
lookup (nam)
     char *nam;
{
  struct sym *sp;
  
  for (sp = symtab; sp != NULL; sp = sp->sy_link)
    if (strncmp (sp->sy_name, nam, NAMMAX) == 0)
      return sp;
  sp = salloc();
  strncpy (sp->sy_name, nam, NAMMAX);
  return sp;
}

struct sym *
salloc ()
{
  struct sym *sp;
  
  sp = (struct sym *)xmalloc(sizeof(struct sym));
  sp->sy_link = symtab;
  symtab = sp;
  sp->sy_expr = NULL;
  sp->sy_value = sp->sy_prime = 0.0;
  sp->sy_sserr = sp->sy_aberr = sp->sy_acerr = 0.0;
  sp->sy_flags = 0;
  return sp;
}

void
sfree (sp)
     struct sym *sp;
{
  if (sp != NULL)
    free ((Voidptr)sp);
}
