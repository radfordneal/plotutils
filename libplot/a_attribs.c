/* This internal method is invoked before drawing any object.  It sets
   the relevant attributes (cap type, join type, line width) to what they
   should be. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* Dash arrays for PS (cylically used, on/off/on/off...), indexed by line
   type.  PS_DASH_ARRAY_LEN is defined in extern.h as 4; do not change it
   without checking the code below (it depends on this value). */
const char _ai_dash_array[NUM_LINE_TYPES][PS_DASH_ARRAY_LEN] =
/* these on/off bit patterns are those used by our X11 driver, and also by
   the xterm Tektronix emulator, except that the emulator seems incorrectly
   to have on and off interchanged */
{
  {0, 0, 0, 0},			/* solid (dummy) */
  {1, 3, 1, 3},			/* dotted */
  {1, 3, 4, 3},			/* dotdashed */
  {4, 4, 4, 4},			/* shortdashed */
  {7, 4, 7, 4}			/* longdashed */
};

void
#ifdef _HAVE_PROTOS
_a_set_attributes (void)
#else
_a_set_attributes ()
#endif
{
  bool changed_width = false;
  double dashbuf[PS_DASH_ARRAY_LEN], scale;
  double desired_ai_line_width = _plotter->drawstate->device_line_width;
  int desired_ai_cap_style = _ps_cap_style[_plotter->drawstate->cap_type];
  int desired_ai_join_style = _ps_join_style[_plotter->drawstate->join_type];
  int desired_ai_line_type = _plotter->drawstate->line_type;  
  int i;

  if (_plotter->ai_cap_style != desired_ai_cap_style)
    {
      sprintf (_plotter->page->point, "%d J\n", desired_ai_cap_style);
      _update_buffer (_plotter->page);
      _plotter->ai_cap_style = desired_ai_cap_style;
    }
  
  if (_plotter->ai_join_style != desired_ai_join_style)
    {
      sprintf (_plotter->page->point, "%d j\n", desired_ai_join_style);
      _update_buffer (_plotter->page);
      _plotter->ai_join_style = desired_ai_join_style;
    }

  if (_plotter->ai_line_width != desired_ai_line_width)
    {
      sprintf (_plotter->page->point, "%.4f w\n", desired_ai_line_width);
      _update_buffer (_plotter->page);
      _plotter->ai_line_width = desired_ai_line_width;
      changed_width = true;
    }

  if (_plotter->ai_line_type != desired_ai_line_type
      || (changed_width && desired_ai_line_type != L_SOLID))
    {
      /* compute PS dash array for this line type (scale by line width
	 if larger than 1 device unit, i.e., 1 point) */
      scale = _plotter->drawstate->device_line_width;
      if (scale < 1.0)
	scale = 1.0;
      for (i = 0; i < PS_DASH_ARRAY_LEN; i++)
	dashbuf[i] = scale * _ps_dash_array[_plotter->drawstate->line_type][i];
      
      /* emit dash array.  THIS ASSUMES PS_DASH_ARRAY_LEN = 4. */
      if (desired_ai_line_type == L_SOLID)
	sprintf (_plotter->page->point, "\
[] 0 d\n");			/* no actual dash array */
      else
        sprintf (_plotter->page->point, "\
[%.4f %.4f %.4f %.4f] 0 d\n", 
                 dashbuf[0], dashbuf[1], dashbuf[2], dashbuf[3]);
      _update_buffer (_plotter->page);
      _plotter->ai_line_type = desired_ai_line_type;
    }

  return;
}
