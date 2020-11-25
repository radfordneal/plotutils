#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_h_erase_page (S___(Plotter *_plotter))
#else
_h_erase_page (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return true;
}
