/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is used for Plotters that emit graphics only after all
   pages of graphics have been drawn, and the Plotter is deleted.  E.g.,
   for PS Plotters; this delayed output allows the output to satisfy the
   DSC (Document Structuring Conventions). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_p_openpl (S___(Plotter *_plotter))
#else
_p_openpl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  plOutbuf *new_page;
  int i;

  if (_plotter->open)
    {
      _plotter->error (R___(_plotter) "openpl: invalid operation");
      return -1;
    }

  /* prepare buffer in which we'll cache code for this page */
  new_page = _new_outbuf ();

  if (_plotter->opened == false) /* first page */
    {
      _plotter->page = new_page;
      /* Save a pointer to the first page, since we'll be caching
	 all pages until the Plotter is deleted. */
      _plotter->first_page = new_page;
    }
  else
    /* add new page to tail of list, update pointer to current page */
    {
      _plotter->page->next = new_page;
      _plotter->page = new_page;
    }

  /* initialize `font used' array(s) for this page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->page->ps_font_used[i] = false;
#ifdef USE_LJ_FONTS_IN_PS
  for (i = 0; i < NUM_PCL_FONTS; i++)
    _plotter->page->pcl_font_used[i] = false;
#endif

  /* invoke generic method, to e.g. create drawing state */
  _g_openpl (S___(_plotter));

  return 0;
}
