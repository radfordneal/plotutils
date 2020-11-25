/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* This file also contains the endsubpath() method, which is a placeholder
   (it is not really implemented yet). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_endpath (S___(Plotter *_plotter))
#else
_m_endpath (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "endpath: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ENDPATH);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_endsubpath (S___(Plotter *_plotter))
#else
_m_endsubpath (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "endsubpath: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_ENDSUBPATH);
  _meta_emit_terminator (S___(_plotter));

  return 0;
}

