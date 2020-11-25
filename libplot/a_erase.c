#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_a_erase_page (S___(Plotter *_plotter))
#else
_a_erase_page (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int i;

  /* The following resetting code should duplicate what is done in
     begin_page(). */

  /* reinitialize `font used' array for this page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;

  /* reset other AIPlotter variables, as if the page had just been opened */
  _plotter->ai_pen_cyan = 0.0;
  _plotter->ai_pen_magenta = 0.0;
  _plotter->ai_pen_yellow = 0.0;
  _plotter->ai_pen_black = 1.0;  
  _plotter->ai_fill_cyan = 0.0;
  _plotter->ai_fill_magenta = 0.0;
  _plotter->ai_fill_yellow = 0.0;
  _plotter->ai_fill_black = 1.0;  
  _plotter->ai_cyan_used = false;
  _plotter->ai_magenta_used = false;
  _plotter->ai_yellow_used = false;
  _plotter->ai_black_used = false;
  _plotter->ai_cap_style = PS_LINE_CAP_BUTT;
  _plotter->ai_join_style = PS_LINE_JOIN_MITER;  
  _plotter->ai_miter_limit = 4.0; /* default value for AI */
  _plotter->ai_line_type = L_SOLID;  
  _plotter->ai_line_width = 1.0;    
  _plotter->ai_fill_rule_type = 0; /* i.e. nonzero winding number rule */

  return true;
}
