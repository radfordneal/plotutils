/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

/* This version is for CGMPlotters.  It draws a point as CGM marker #1. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_c_fpoint (R___(Plotter *_plotter) double x, double y)
#else
_c_fpoint (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  double xd, yd;
  int i_x, i_y;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fpoint: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (_plotter->cgm_marker_type != CGM_M_DOT)
  /* emit "MARKER TYPE" command */
    {
      int byte_count, data_byte_count, data_len;
      
      data_len = 2;		/* number of bytes per index */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 6,
				data_len, &byte_count,
				"MARKERTYPE");
      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
		       CGM_M_DOT,
		       data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update marker type */
      _plotter->cgm_marker_type = CGM_M_DOT;
  }
  
  /* N.B. Should we set the marker size as well? Any good CGM interpreter
     should draw a dot marker as a very small dot, perhaps a single
     pixel. */

  /* set CGM marker color */
  _plotter->drawstate->cgm_object_type = CGM_OBJECT_MARKER;/* pass hint */
  _plotter->set_pen_color (S___(_plotter));

  /* compute location in device frame */
  xd = XD(x, y);
  yd = YD(x, y);
  i_x = IROUND(xd);
  i_y = IROUND(yd);

  /* emit "POLYMARKER" command, to draw a single marker */
  {
    int byte_count, data_byte_count, data_len;
    
    data_len = 1 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
    byte_count = data_byte_count = 0;
    _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
			      CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 3,
			      data_len, &byte_count,
			      "MARKER");
    _cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
		     i_x, i_y,
		     data_len, &data_byte_count, &byte_count);
    _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				  &byte_count);
  }
  
  return 0;
}
