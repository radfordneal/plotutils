/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for Plotters that do not plot in real time, but emit a
   page of graphics when closepl() is called. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_a_openpl (void)
#else
_a_openpl ()
#endif
{
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* Prepare buffer in which we'll cache graphics code for this page.
     Although we won't be emitting graphics in real time, we don't maintain
     a linked list of buffers, one per page; we don't need to. */
  _plotter->page = _new_outbuf ();
  
  /* With each call to openpl(), we reset our knowledge of Illustrator's
     internal state, i.e. the dynamic AI-specific data members of the
     AIPlotter.  The values are the same as are used in initializing the
     AIPlotter (see a_defplot.c). */
     
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
  _plotter->ai_cap_style = PS_CAP_BUTT;
  _plotter->ai_join_style = PS_JOIN_MITER;  
  _plotter->ai_miter_limit = 4.0; /* default value for AI */
  _plotter->ai_line_type = L_SOLID;  
  _plotter->ai_line_width = 1.0;    
  _plotter->ai_fill_rule_type = 0; /* i.e. nonzero winding number rule */

  /* invoke generic method, to e.g. create a drawing state */
  _g_openpl ();

  return 0;
}
