/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This generic version doesn't prepare any output buffer, since for
   GenericPlotters, no graphics code will actually be emitted. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_openpl (void)
#else
_g_openpl ()
#endif
{
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  /* frames in page are numbered starting with zero */
  _plotter->frame_number = 0;

  return 0;
}
