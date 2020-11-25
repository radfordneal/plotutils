/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r. */

/* This file also contains the ellipse method, which is a GNU extension to
   libplot.  It draws an object: an ellipse with center xc,yc and semi-axes
   of length rx and ry (the former at a specified angle with the x-axis). */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_c_fcircle (R___(Plotter *_plotter) double x, double y, double radius)
#else
_c_fcircle (R___(_plotter) x, y, radius)
     S___(Plotter *_plotter;)
     double x, y, radius;
#endif
{
  int byte_count, data_byte_count, data_len;
  int desired_interior_style;
  const char *desired_interior_style_string;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fcircle: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    {
      _plotter->drawstate->pos.x = x; /* move to center of circle */
      _plotter->drawstate->pos.y = y;
      return 0;
    }

  /* if `disconnected' line type, don't draw anything (libplot convention) */

  if (_plotter->drawstate->points_are_connected)
    {
      double xd, yd, radius_d;
      int i_x, i_y, i_radius;		/* center and radius, quantized */

      /* if affine map from user frame to device frame is anisotropic, draw
         circle as an ellipse */
      if (!_plotter->drawstate->transform.uniform)
	return _plotter->fellipse (R___(_plotter) x, y, radius, radius, 0.0);

      /* a circle in device frame, so compute center and radius */
      xd = XD(x, y);
      yd = YD(x, y);
      radius_d = sqrt(XDV(radius,0)*XDV(radius,0)+YDV(radius,0)*YDV(radius,0));
      i_x = IROUND(xd);
      i_y = IROUND(yd);
      i_radius = IROUND(radius_d);

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
	/* should edge the circle */
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
      
      /* emit "CIRCLE" command */
      {
	data_len = 3 * CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 12,
				  data_len, &byte_count,
				  "CIRCLE");
	_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
			 i_x, i_y,
			 data_len, &data_byte_count, &byte_count);
	_cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			   i_radius,
			   data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				      &byte_count);
      }
    }

  _plotter->drawstate->pos.x = x; /* move to center of circle */
  _plotter->drawstate->pos.y = y;

  return 0;
}

int
#ifdef _HAVE_PROTOS
_c_fellipse (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle)
#else
_c_fellipse (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;)
     double x, y, rx, ry, angle;
#endif
{
  int byte_count, data_byte_count, data_len;
  int desired_interior_style;
  const char *desired_interior_style_string;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "fellipse: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->points_in_path > 0)
    _plotter->endpath (S___(_plotter)); /* flush polyline if any */

  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    {
      _plotter->drawstate->pos.x = x; /* move to center of ellipse */
      _plotter->drawstate->pos.y = y;
      return 0;
    }

  /* if `disconnected' line type, don't draw anything (libplot convention) */

  if (_plotter->drawstate->points_are_connected)
    {
      double xd, yd;		/* center, in device frame */
      int i_x, i_y;		/* center, quantized */
      double theta, costheta, sintheta;
      double cd1_endpoint_x, cd1_endpoint_y; /* conjugate diameter endpts */
      double cd2_endpoint_x, cd2_endpoint_y;
      int i1_x, i1_y, i2_x, i2_y; /* same, quantized */

      /* compute center, in device frame */
      xd = XD(x, y);
      yd = YD(x, y);
      i_x = IROUND(xd);
      i_y = IROUND(yd);

      /* inclination angle (radians), in user frame */
      theta = M_PI * angle / 180.0;
      costheta = cos (theta);
      sintheta = sin (theta);

      /* perform affine user->device coor transformation, computing
	 endpoints of conjugate diameter pair, in device frame */
      cd1_endpoint_x = XD(x + rx * costheta, y + rx * sintheta);
      cd1_endpoint_y = YD(x + rx * costheta, y + rx * sintheta);
      cd2_endpoint_x = XD(x - ry * sintheta, y + ry * costheta);
      cd2_endpoint_y = YD(x - ry * sintheta, y + ry * costheta);
      i1_x = IROUND(cd1_endpoint_x);
      i1_y = IROUND(cd1_endpoint_y);
      i2_x = IROUND(cd2_endpoint_x);
      i2_y = IROUND(cd2_endpoint_y);

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
	/* should edge the ellipse */
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
      
      /* emit "ELLIPSE" command */
      {
	data_len = 3 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 17,
				  data_len, &byte_count,
				  "ELLIPSE");
	_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
			 i_x, i_y,
			 data_len, &data_byte_count, &byte_count);
	_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
			 i1_x, i1_y,
			 data_len, &data_byte_count, &byte_count);
	_cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
			 i2_x, i2_y,
			 data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				      &byte_count);
      }
    }

  _plotter->drawstate->pos.x = x; /* move to center of ellipse */
  _plotter->drawstate->pos.y = y;

  return 0;
}
