/* This file contains output buffer manipulation routines, which are called
   by functions in versions of libplot that for one reason or another do
   not support real-time output.  Instead, graphics objects are stored in
   an output buffer, which is written to the output stream when closepl()
   is called.  This is a kludge, which may eventually be replaced by an
   in-core object hierarchy, which closepl() will scan. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* initial length for an output buffer, for holding output strings (should
   be large enough to handle any single one of our sprintf's without
   overflow). */

#define INITIAL_OUTBUF_LEN 256

void
#ifdef _HAVE_PROTOS
_initialize_buffer (Outbuffer *bufp)
#else
_initialize_buffer (bufp)
     Outbuffer *bufp;
#endif
{
  bufp->base = (char *)_plot_xmalloc(INITIAL_OUTBUF_LEN * sizeof(char));
  bufp->len = INITIAL_OUTBUF_LEN;
  bufp->current = bufp->base;
  bufp->contents = 0;
  *(bufp->base) = '\0';
}

/* UPDATE_BUFFER must be called after each sprintf() and other object write
   operation */

void
#ifdef _HAVE_PROTOS
_update_buffer (Outbuffer *bufp)
#else
_update_buffer (bufp)
     Outbuffer *bufp;
#endif
{
  int additional;

  additional = strlen (bufp->current);
  bufp->current += additional;
  bufp->contents += additional;

  if (bufp->contents > bufp->len - 1) /* need room for NUL */
    {
      fprintf (stderr, "libplot: output buffer overrun\n");
      exit (1);
    }
  if (bufp->contents > bufp->len / 2)
    {
      bufp->base = 
	(char *)_plot_xrealloc (bufp->base, 2 * bufp->len * sizeof(char));
      bufp->len *= 2;
      bufp->current = bufp->base + bufp->contents;
    }      
}

void
#ifdef _HAVE_PROTOS
_reset_buffer (Outbuffer *bufp)
#else
_reset_buffer (bufp)
     Outbuffer *bufp;
#endif
{
  bufp->current = bufp->base;
  bufp->contents = 0;
  *(bufp->base) = '\0';
}
