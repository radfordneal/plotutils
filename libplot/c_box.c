/* This file contains the box method, which is a standard part of libplot.
   It draws an object: an upright rectangle with diagonal corners x0,y0 and
   x1,y1. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_c_fbox (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
#else
_c_fbox (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;)
     double x0, y0, x1, y1;
#endif
{
  int xd0, xd1, yd0, yd1;	/* in integer device coordinates */
  double xnew, ynew;
  int byte_count, data_byte_count, data_len;
  int desired_interior_style;
  const char *desired_interior_style_string;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fbox: invalid operation");
      return -1;
    }

  /* if user coors -> device coors transformation does not preserve axis
     directions, use generic method to draw a closed polyline */

  if (!_plotter->drawstate->transform.axes_preserved)
    return _g_fbox (R___(_plotter) x0, y0, x1, y1);

  /* otherwise use CGM's box-drawing facility (which aligns boxes with the
     VDC axes) */

  _plotter->endpath (S___(_plotter)); /* flush polyline if any */
  
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    {
      /* move to center (libplot convention) */
      xnew = 0.5 * (x0 + x1);
      ynew = 0.5 * (y0 + y1);
      _plotter->drawstate->pos.x = xnew;
      _plotter->drawstate->pos.y = ynew;
      return 0;
    }

  /* compute corners in device coors */
  xd0 = IROUND(XD(x0, y0));
  yd0 = IROUND(YD(x0, y0));  
  xd1 = IROUND(XD(x1, y1));
  yd1 = IROUND(YD(x1, y1));  

  /* set CGM edge color and attributes, by emitting appropriate commands */
  _plotter->drawstate->cgm_object_type = CGM_OBJECT_CLOSED;/* pass hint */
  _plotter->set_pen_color (S___(_plotter));
  _plotter->set_fill_color (S___(_plotter));
  _plotter->set_attributes (S___(_plotter));

  if (_plotter->drawstate->fill_type == 0)
    /* won't do filling */
    {
      desired_interior_style = CGM_INT_STYLE_EMPTY;
      desired_interior_style_string = "empty";
    }
  else
    /* will do filling */
    {
      desired_interior_style = CGM_INT_STYLE_SOLID;
      desired_interior_style_string = "solid";
    }
  
  if (_plotter->cgm_interior_style != desired_interior_style)
    /* emit "INTERIOR STYLE" command */
    {
      data_len = 2;	/* 2 bytes per enum */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 22,
				data_len, &byte_count,
				"INTSTYLE");
      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
		      desired_interior_style,
		      data_len, &data_byte_count, &byte_count,
		      desired_interior_style_string);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);
      /* update interior style */
      _plotter->cgm_interior_style = desired_interior_style;
    }
  
  if (_plotter->drawstate->pen_type)
    /* should edge the rectangle */
    {
      if (_plotter->cgm_edge_is_visible != true)
	/* emit "EDGE VISIBILITY" command */
	{
	  data_len = 2;	/* 2 bytes per enum */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 30,
				    data_len, &byte_count,
				    "EDGEVIS");
	  _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			  1,
			  data_len, &data_byte_count, &byte_count,
			  "on");
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update edge visibility */
	  _plotter->cgm_edge_is_visible = true;
	}
    }
  else
    /* shouldn't edge the rectangle */
    {
      if (_plotter->cgm_edge_is_visible != false)
	/* emit "EDGE VISIBILITY" command */
	{
	  data_len = 2;	/* 2 bytes per enum */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 30,
				    data_len, &byte_count,
				    "EDGEVIS");
	  _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
			  0,
			  data_len, &data_byte_count, &byte_count,
			  "off");
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update edge visibility */
	  _plotter->cgm_edge_is_visible = false;
	}
    }

  /* emit "RECTANGLE" command */
  {
    data_len = 2 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
    byte_count = data_byte_count = 0;
    _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
			      CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 11,
			      data_len, &byte_count,
			      "RECT");
    _cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
		     xd0, yd0,
		     data_len, &data_byte_count, &byte_count);
    _cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
		     xd1, yd1,
		     data_len, &data_byte_count, &byte_count);
    _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				  &byte_count);
  }

  /* move to center (libplot convention) */
  xnew = 0.5 * (x0 + x1);
  ynew = 0.5 * (y0 + y1);
  _plotter->drawstate->pos.x = xnew;
  _plotter->drawstate->pos.y = ynew;

  return 0;
}
