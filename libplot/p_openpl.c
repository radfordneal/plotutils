/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object.

   For PSPlotter objects, we initialize the line type and choice of font,
   and initialize the buffer in which we'll store Postscript code.  We
   don't print out the postscript prologue, since we'll know the bounding
   box only after all objects have been plotted.  The closepl() method will
   write everything out.

   We also determine the page size and the location on the page of the
   graphics display, so that we'll be able to work out the map from user
   coordinates to device coordinates in space.c. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_p_openpl (void)
#else
_p_openpl ()
#endif
{
  const char *length_s, *pagesize;
  const Pagedata *pagedata;
  int i;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* initialize certain data members from values of relevant class variables */

  length_s = (const char *)_get_plot_param ("MAX_LINE_LENGTH");
  {
    int local_length;
    
    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	_plotter->error ("bad MAX_LINE_LENGTH parameter, can't initialize");
	return -1;
      }
    else
      _plotter->max_unfilled_polyline_length = local_length;
  }

  /* determine page type i.e. determine the range of device coordinates
   over which the graphics display will extend (and hence the
   transformation from user to device coordinates). */
  pagesize = (const char *)_get_plot_param ("PAGESIZE");
  pagedata = _pagetype(pagesize);
  if (pagedata == NULL)
    {
      _plotter->error ("bad PAGESIZE variable, can't initialize");
      return -1;
    }
  _plotter->display_coors = pagedata->ps;

  /* prepare buffer in which we'll cache all Postscript code */
  _initialize_buffer (&_plotter->outbuf);

  /* reset range bounds (i.e. BoundingBox) */
  _reset_range();

  /* initialize `font used' array */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->ps_font_used[i] = false;

  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}
