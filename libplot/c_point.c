/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

/* This version is for CGMPlotters.  It draws a point as CGM marker #1. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_c_paint_point (S___(Plotter *_plotter))
#else
_c_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xd, yd;
  int i_x, i_y;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      if (_plotter->cgm_marker_type != CGM_M_DOT)
	/* emit "MARKER TYPE" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 2;		/* number of bytes per index */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 6,
				    data_len, &byte_count,
				    "MARKERTYPE");
	  _cgm_emit_index (_plotter->data->page, false, _plotter->cgm_encoding,
			   CGM_M_DOT,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);

	  /* update marker type */
	  _plotter->cgm_marker_type = CGM_M_DOT;
	}
  
      /* N.B. Should we set the marker size as well? Any good CGM
	 interpreter should draw a dot marker as a very small dot, perhaps
	 a single pixel. */

      /* set CGM marker color */
      _c_set_pen_color (R___(_plotter) CGM_OBJECT_MARKER);

      /* compute location in device frame */
      xd = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yd = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      i_x = IROUND(xd);
      i_y = IROUND(yd);
      
      /* emit "POLYMARKER" command, to draw a single marker */
      {
	int byte_count, data_byte_count, data_len;
	
	data_len = 1 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 3,
				  data_len, &byte_count,
				  "MARKER");
	_cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			 i_x, i_y,
			 data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
				      &byte_count);
      }
    }
}
