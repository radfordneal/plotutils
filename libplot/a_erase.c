/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_a_erase (S___(Plotter *_plotter))
#else
_a_erase (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int i;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "erase: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  /* if we have an output buffer, i.e., if we're not plotting in real time,
     discard all objects */
  if (_plotter->page)
    _reset_outbuf (_plotter->page);

  /* reinitialize `font used' array for this page */
  for (i = 0; i < NUM_PS_FONTS; i++)
    _plotter->page->ps_font_used[i] = false;

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

  /* on to next frame */
  _plotter->frame_number++;

  return 0;
}
