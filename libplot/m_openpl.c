/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_openpl (S___(Plotter *_plotter))
#else
_m_openpl (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  bool not_previously_opened;

  if (_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "openpl: invalid operation");
      return -1;
    }

  /* flag device as open */
  _plotter->open = true;
  not_previously_opened = _plotter->opened ? false : true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  /* space() not invoked yet, to set the user frame->device frame map */
  _plotter->space_invoked = false;

  if (not_previously_opened)
    /* emit metafile header, i.e. magic string */
    {
      _plotter->write_string (R___(_plotter) PLOT_MAGIC);

      /* format type 1 = GNU binary, type 2 = GNU portable */
      if (_plotter->meta_portable_output)
	_plotter->write_string (R___(_plotter) " 2\n");
      else
	_plotter->write_string (R___(_plotter) " 1\n");
    }

  _meta_emit_byte (R___(_plotter) (int)O_OPENPL);
  _meta_emit_terminator (S___(_plotter));
  
  /* create drawing state, add it as the first member of the linked list;
     use generic method because we don't want to emit an op code */
  _g_savestate (S___(_plotter));

  /* frames in page are numbered starting with zero */
  _plotter->frame_number = 0;

  return 0;
}
