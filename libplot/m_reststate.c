/* This file contains the restorestate method, which is a GNU extension to
   libplot.  It pops off the drawing state on the top of the stack of
   drawing states.  Drawing states (other than the one which is always
   present, and may not be popped off) are created and pushed onto the
   stack by invoking the savestate routine. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_restorestate(S___(Plotter *_plotter))
#else
_m_restorestate(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "restorestate: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_RESTORESTATE);
  _meta_emit_terminator (S___(_plotter));

  /* invoke generic method */
  return _g_restorestate (S___(_plotter));
}
