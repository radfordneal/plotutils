#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_t_end_page (S___(Plotter *_plotter))
#else
_t_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  _tek_move (R___(_plotter) 0, 0); /* go to lower left corner in Tek space */
  _tek_mode (R___(_plotter) MODE_ALPHA); /* switch to alpha mode */

  switch (_plotter->tek_display_type) /* exit from Tek mode */
    {
    case D_KERMIT:
      /* use VT340 command to exit graphics mode */
      _write_string (_plotter->data, "\033[?38l");
      /* following command may be an alternative */
      /*
	_write_string (_plotter->data, "\030");
      */
      break;
    case D_XTERM:
      /* ESC C-c, restore to VT102 mode */
      _write_string (_plotter->data, "\033\003"); 
      break;
    case D_GENERIC:
    default:
      break;
    }

  return true;
}
