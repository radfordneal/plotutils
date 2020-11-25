/* This file contains routines for creating, manipulating, and deleting a
   special sort of output buffer: an Outbuf.  They are invoked by drawing
   methods for Plotters that do not do real-time output.  Such Plotters
   store the device code for each page of graphics in an Outbuf.  Along
   with a page of device code, an Outbuf optionally stores bounding box
   information for the page.

   Outbufs are resized when they are too full.  The strange resizing method
   (_UPDATE_BUFFER) is needed because on many systems, sprintf() does not
   return the number of characters it writes.  _UPDATE_BUFFER is called
   after each call to sprintf(); it is not invoked automatically.
   
   Output buffers of this sort are a bit of a kludge.  They may eventually
   be replaced or supplemented by an in-core object hierarchy, which
   deletepl() will scan.

   When erase() is invoked on the Plotter, _RESET_OUTBUF is called, to
   remove all graphics from the Outbuf.  There is provision for keeping a
   section of initialization code in the Outbuf, untouched.  This is
   arranged by a previous call to _FREEZE_OUTBUF.  Anything in the Outbuf
   at that time will be untouched by a later call to _RESET_OUTBUF. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* initial length for an Outbuf (should be large enough to handle any
   single one of our sprintf's [including final NUL] without overflow). */
#define INITIAL_OUTBUF_LEN 256

Outbuf *
#ifdef _HAVE_PROTOS
_new_outbuf (void)
#else
_new_outbuf ()
#endif
{
  Outbuf *bufp;

  bufp = (Outbuf *)_plot_xmalloc(sizeof(Outbuf));
  bufp->base = (char *)_plot_xmalloc(INITIAL_OUTBUF_LEN * sizeof(char));
  bufp->len = (unsigned long)INITIAL_OUTBUF_LEN;
  bufp->next = NULL;
  bufp->reset_point = bufp->base;
  bufp->reset_contents = (unsigned long)0L;
  _reset_outbuf (bufp);

  return bufp;
}

void
#ifdef _HAVE_PROTOS
_reset_outbuf (Outbuf *bufp)
#else
_reset_outbuf (bufp)
     Outbuf *bufp;
#endif
{
  *(bufp->reset_point) = '\0';
  bufp->point = bufp->reset_point;
  bufp->contents = bufp->reset_contents;

  bufp->xrange_min = DBL_MAX;
  bufp->xrange_max = -(DBL_MAX);
  bufp->yrange_min = DBL_MAX;
  bufp->yrange_max = -(DBL_MAX);
}

void
#ifdef _HAVE_PROTOS
_freeze_outbuf (Outbuf *bufp)
#else
_freeze_outbuf (bufp)
     Outbuf *bufp;
#endif
{
  bufp->reset_point = bufp->point;
  bufp->reset_contents = bufp->contents;
}

void
#ifdef _HAVE_PROTOS
_delete_outbuf (Outbuf *bufp)
#else
_delete_outbuf (bufp)
     Outbuf *bufp;
#endif
{
  if (bufp)
    {
      free (bufp->base);
      free (bufp);
    }
}

/* UPDATE_BUFFER is called manually, after each sprintf() and other object
   write operations */
void
#ifdef _HAVE_PROTOS
_update_buffer (Outbuf *bufp)
#else
_update_buffer (bufp)
     Outbuf *bufp;
#endif
{
  int additional;

  /* determine how many add'l chars were added */
  additional = strlen (bufp->point);
  bufp->point += additional;
  bufp->contents += additional;
  
  if (bufp->contents + 1 > bufp->len) /* need room for NUL */
    {
      fprintf (stderr, "libplot: output buffer overrun\n");
      exit (1);
    }
  if (bufp->contents > (bufp->len >> 1))
    /* expand buffer */
    {
      bufp->base = 
	(char *)_plot_xrealloc (bufp->base, 2 * bufp->len * sizeof(char));
      bufp->len *= 2;
      bufp->point = bufp->base + bufp->contents;
      bufp->reset_point = bufp->base + bufp->reset_contents;
    }      
}

/* query bounding box information for the page */
void 
#ifdef _HAVE_PROTOS
_get_range (Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
#else
_get_range (bufp, xmin, xmax, ymin, ymax)
     Outbuf *bufp;
     double *xmin, *xmax, *ymin, *ymax;
#endif
{
  *xmax = bufp->xrange_max;
  *xmin = bufp->xrange_min;
  *ymax = bufp->yrange_max;
  *ymin = bufp->yrange_min;
}

/* update bounding box information for the page, to take account of a point
   being plotted */
void 
#ifdef _HAVE_PROTOS
_set_range (Outbuf *bufp, double x, double y)
#else
_set_range (bufp, x, y)
     Outbuf *bufp;
     double x, y;
#endif
{
  if (x > bufp->xrange_max) bufp->xrange_max = x;
  if (x < bufp->xrange_min) bufp->xrange_min = x;
  if (y > bufp->yrange_max) bufp->yrange_max = y;
  if (y < bufp->yrange_min) bufp->yrange_min = y;
}
