#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_s_begin_page (S___(Plotter *_plotter))
#else
_s_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* copy background color to the SVG-specific part of the SVGPlotter; that
     value will be written out at the head of the page (also, it'll be
     updated before that, if erase() is invoked) */
  _plotter->s_bgcolor = _plotter->drawstate->bgcolor;
  _plotter->s_bgcolor_suppressed = _plotter->drawstate->bgcolor_suppressed;

  return true;
}
