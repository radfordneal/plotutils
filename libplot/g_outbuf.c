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
  int i;

  *(bufp->reset_point) = '\0';
  bufp->point = bufp->reset_point;
  bufp->contents = bufp->reset_contents;

  /* initialize bounding box to an empty (self-contradictory) box */
  bufp->xrange_min = DBL_MAX;
  bufp->xrange_max = -(DBL_MAX);
  bufp->yrange_min = DBL_MAX;
  bufp->yrange_max = -(DBL_MAX);

  /* initialize `font used' arrays for the page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    bufp->ps_font_used[i] = false;
  for (i = 0; i < NUM_PCL_FONTS; i++)
    bufp->pcl_font_used[i] = false;
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
      exit (EXIT_FAILURE);
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

/* update bounding box information for an outbuf, to take account of a
   point being plotted on the associated page */
void 
#ifdef _HAVE_PROTOS
_update_bbox (Outbuf *bufp, double x, double y)
#else
_update_bbox (bufp, x, y)
     Outbuf *bufp;
     double x, y;
#endif
{
  if (x > bufp->xrange_max) bufp->xrange_max = x;
  if (x < bufp->xrange_min) bufp->xrange_min = x;
  if (y > bufp->yrange_max) bufp->yrange_max = y;
  if (y < bufp->yrange_min) bufp->yrange_min = y;
}

/* return bounding box information for an outbuf */
void 
#ifdef _HAVE_PROTOS
_bbox_of_outbuf (Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
#else
_bbox_of_outbuf (bufp, xmin, xmax, ymin, ymax)
     Outbuf *bufp;
     double *xmin, *xmax, *ymin, *ymax;
#endif
{
  double page_x_min = DBL_MAX;
  double page_y_min = DBL_MAX;  
  double page_x_max = -(DBL_MAX);
  double page_y_max = -(DBL_MAX);  

  if (bufp)
    {
      page_x_max = bufp->xrange_max;
      page_x_min = bufp->xrange_min;
      page_y_max = bufp->yrange_max;
      page_y_min = bufp->yrange_min;
    }

  *xmin = page_x_min;
  *ymin = page_y_min;
  *xmax = page_x_max;
  *ymax = page_y_max;
}

/* compute bounding box information for a linked list of outbufs
   (i.e. pages), starting with a specified outbuf (i.e., page) */
void 
#ifdef _HAVE_PROTOS
_bbox_of_outbufs (Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
#else
_bbox_of_outbufs (bufp, xmin, xmax, ymin, ymax)
     Outbuf *bufp;
     double *xmin, *xmax, *ymin, *ymax;
#endif
{
  double doc_x_min = DBL_MAX;
  double doc_y_min = DBL_MAX;  
  double doc_x_max = -(DBL_MAX);
  double doc_y_max = -(DBL_MAX);  
  double page_x_min, page_x_max, page_y_min, page_y_max;
  Outbuf *page = bufp;

  while (page)
    {
      page_x_max = page->xrange_max;
      page_x_min = page->xrange_min;
      page_y_max = page->yrange_max;
      page_y_min = page->yrange_min;

      if (!((page_x_max < page_x_min || page_y_max < page_y_min)))
	/* nonempty page */
	{
	  if (page_x_max > doc_x_max) doc_x_max = page_x_max;
	  if (page_y_max > doc_y_max) doc_y_max = page_y_max;
	  if (page_x_min < doc_x_min) doc_x_min = page_x_min;
	  if (page_y_min < doc_y_min) doc_y_min = page_y_min;
	}
      page = page->next;
    }

  *xmin = doc_x_min;
  *ymin = doc_y_min;
  *xmax = doc_x_max;
  *ymax = doc_y_max;
}
