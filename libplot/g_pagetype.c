/* This file contains a utility function, _pagetype(), which is used to
   search the database of known pagetypes for the one specified in the
   PAGESIZE parameter. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_pagetype.h"

const Pagedata *
#ifdef _HAVE_PROTOS
_pagetype(const char *name)
#else
_pagetype(name)
     const char *name;
#endif
{
  const Pagedata *pagedata = _pagedata;
  int i;
  
  for (i = 0; i < NUM_PAGESIZES; i++, pagedata++)
    if (strcasecmp (pagedata->name, name) == 0)
      return pagedata;

  return (Pagedata *)NULL;	/* didn't find pagetype */
}
