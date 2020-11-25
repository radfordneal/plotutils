/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a marker, of specified size, at a
   specified location.  This marker may be one of a list of standard
   symbols, or a single character in the current font.

   The `size' argument is expressed in terms of user coordinates.  If the
   marker is a character, it is the font size (i.e., the em size of the
   font).  If the marker is a symbol, the maximum dimension of the symbol
   will be a fixed fraction of `size'. */

#include "sys-defines.h"
#include "extern.h"

/* The maximum dimension of most markers, e.g. the diameter of the circle
   marker (marker #4).  Expressed as a fraction of the `size' argument. */
#define MAXIMUM_MARKER_DIMENSION (5.0/8.0)

int
#ifdef _HAVE_PROTOS
_c_fmarker (R___(Plotter *_plotter) double x, double y, int type, double size)
#else
_c_fmarker (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;)
     double x, y;
     int type;
     double size;
#endif
{
  int desired_marker_type, desired_marker_size;
  double xd, yd, size_d;
  int i_x, i_y;

  switch (type)
    {
    case M_DOT:
      desired_marker_type = CGM_M_DOT;
      break;
    case M_PLUS:
      desired_marker_type = CGM_M_PLUS;
      break;
    case M_ASTERISK:
      desired_marker_type = CGM_M_ASTERISK;
      break;
    case M_CIRCLE:
      desired_marker_type = CGM_M_CIRCLE;
      break;
    case M_CROSS:
      desired_marker_type = CGM_M_CROSS;
      break;
    default:
      return _g_fmarker (R___(_plotter) x, y, type, size);
      break;
    }

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fmarker: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */


  if (_plotter->cgm_marker_type != desired_marker_type)
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
		       desired_marker_type,
		       data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update marker type */
      _plotter->cgm_marker_type = desired_marker_type;
  }
  
  /* compute size of marker in device frame */
  size_d = sqrt(XDV(size,0)*XDV(size,0)+YDV(size,0)*YDV(size,0));
  desired_marker_size = IROUND(MAXIMUM_MARKER_DIMENSION * size_d);

  if (desired_marker_type != CGM_M_DOT 
      && _plotter->cgm_marker_size != desired_marker_size)
  /* emit "MARKER SIZE" command (for a dot we don't bother, since dots are
     meant to be drawn as small as possible) */
    {
      int byte_count, data_byte_count, data_len;
      
      data_len = CGM_BINARY_BYTES_PER_INTEGER;
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 7,
				data_len, &byte_count,
				"MARKERSIZE");
      _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			 desired_marker_size,
			 data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update marker size */
      _plotter->cgm_marker_size = desired_marker_size;
    }
  
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

