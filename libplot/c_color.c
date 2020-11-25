/* This file contains device-specific color database access routines.
   These routines are called by various CGMPlotter methods, before drawing
   objects.

   The CGM output file will use either 24-bit RGB or 48-bit RGB, depending
   on the value of CGM_BINARY_BYTES_PER_COLOR_COMPONENT (set in extern.h; 
   1 or 2, respectively).  This code assumes that the value is 1 or 2, even
   though CGM files allow 3 or 4 as well.  To handle the `4' case, we
   should rewrite this to use unsigned ints rather than signed ints.

   The reason we don't bother with 3 or 4 is that internally, libplot uses
   48-bit color.  So 48-bit RGB in the CGM output file is all we need. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_c_set_pen_color(S___(Plotter *_plotter))
#else
_c_set_pen_color(S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int red_long, green_long, blue_long;
  int red, green, blue;
  int byte_count, data_byte_count, data_len;
  int fullstrength;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->fgcolor.red;
  green_long = _plotter->drawstate->fgcolor.green;
  blue_long = _plotter->drawstate->fgcolor.blue;

  /* 24-bit or 48-bit RGB (as used in CGMs) */
  switch (CGM_BINARY_BYTES_PER_COLOR_COMPONENT)
    {
    case 1:
      /* 24-bit */
      red = (((unsigned int)red_long) >> 8) & 0xff;
      green = (((unsigned int)green_long) >> 8) & 0xff;
      blue = (((unsigned int)blue_long) >> 8) & 0xff;
      break;
    case 2:
    default:
      /* 48-bit */
      red = red_long;
      green = green_long;
      blue = blue_long;
      break;
    }

  fullstrength = (1 << (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT)) - 1;
  if ((red != 0 || green != 0 || blue != 0)
      && (red != fullstrength || green != fullstrength || blue != fullstrength))
    _plotter->cgm_page_need_color = true;

  switch (_plotter->drawstate->cgm_object_type)
    {
    case CGM_OBJECT_OPEN:
      if (_plotter->cgm_line_color.red != red 
	  || _plotter->cgm_line_color.green != green
	  || _plotter->cgm_line_color.blue != blue)
	/* emit "LINE_COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 4,
				    data_len, &byte_count,
				    "LINECOLR");
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's pen color */
	  _plotter->cgm_line_color.red = red;
	  _plotter->cgm_line_color.green = green;
	  _plotter->cgm_line_color.blue = blue;
	}
      break;
    case CGM_OBJECT_CLOSED:
      if (_plotter->cgm_edge_color.red != red 
	  || _plotter->cgm_edge_color.green != green
	  || _plotter->cgm_edge_color.blue != blue)
	/* emit "EDGE_COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 29,
				    data_len, &byte_count,
				    "EDGECOLR");
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's edge color */
	  _plotter->cgm_edge_color.red = red;
	  _plotter->cgm_edge_color.green = green;
	  _plotter->cgm_edge_color.blue = blue;
	}
      break;
    case CGM_OBJECT_MARKER:
      if (_plotter->cgm_marker_color.red != red 
	  || _plotter->cgm_marker_color.green != green
	  || _plotter->cgm_marker_color.blue != blue)
	/* emit "MARKER COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 8,
				    data_len, &byte_count,
				    "MARKERCOLR");
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's marker color */
	  _plotter->cgm_marker_color.red = red;
	  _plotter->cgm_marker_color.green = green;
	  _plotter->cgm_marker_color.blue = blue;
	}
      break;
    case CGM_OBJECT_TEXT:
      if (_plotter->cgm_text_color.red != red 
	  || _plotter->cgm_text_color.green != green
	  || _plotter->cgm_text_color.blue != blue)
	/* emit "TEXT COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 14,
				    data_len, &byte_count,
				    "TEXTCOLR");
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's text color */
	  _plotter->cgm_text_color.red = red;
	  _plotter->cgm_text_color.green = green;
	  _plotter->cgm_text_color.blue = blue;
	}
      break;
    default:
      break;
    }
}

void
#ifdef _HAVE_PROTOS
_c_set_fill_color(S___(Plotter *_plotter))
#else
_c_set_fill_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int red_long, green_long, blue_long;
  int red, green, blue;
  double red_d, green_d, blue_d;
  double desaturate;
  int fullstrength;
  int byte_count, data_byte_count, data_len;

  if (_plotter->drawstate->fill_type == 0)
    /* don't do anything, fill color will be ignored when writing objects*/
    return;

  if (_plotter->drawstate->cgm_object_type != CGM_OBJECT_OPEN
      && _plotter->drawstate->cgm_object_type != CGM_OBJECT_CLOSED)
    /* don't do anything; won't be filling */
    return;

  /* scale each RGB from a 16-bit quantity to range [0.0,1.0] */
  red_d = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
  green_d = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
  blue_d = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;

  /* fill_type, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_plotter->drawstate->fill_type - 1.)/0xFFFE;
  red_d = red_d + desaturate * (1.0 - red_d);
  green_d = green_d + desaturate * (1.0 - green_d);
  blue_d = blue_d + desaturate * (1.0 - blue_d);

  /* restore each RGB to a 16-bit quantity (48 bits in all) */
  red_long = IROUND(0xFFFF * red_d);
  green_long = IROUND(0xFFFF * green_d);
  blue_long = IROUND(0xFFFF * blue_d);

  /* 24-bit or 48-bit RGB (as used in CGMs) */
  switch (CGM_BINARY_BYTES_PER_COLOR_COMPONENT)
    {
    case 1:
      /* 24-bit */
      red = (((unsigned int)red_long) >> 8) & 0xff;
      green = (((unsigned int)green_long) >> 8) & 0xff;
      blue = (((unsigned int)blue_long) >> 8) & 0xff;
      break;
    case 2:
    default:
      /* 48-bit */
      red = red_long;
      green = green_long;
      blue = blue_long;
      break;
    }

  fullstrength = (1 << (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT)) - 1;
  if ((red != 0 || green != 0 || blue != 0)
      && (red != fullstrength || green != fullstrength || blue != fullstrength))
    _plotter->cgm_page_need_color = true;

  if (_plotter->cgm_fillcolor.red != red 
      || _plotter->cgm_fillcolor.green != green
      || _plotter->cgm_fillcolor.blue != blue)
    /* emit "FILL COLOR" command */
    {
      data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 23,
				data_len, &byte_count,
				"FILLCOLR");
      _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				 (unsigned int)red,
				 data_len, &data_byte_count, &byte_count);
      _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				 (unsigned int)green,
				 data_len, &data_byte_count, &byte_count);
      _cgm_emit_color_component (_plotter->page, false, _plotter->cgm_encoding,
				 (unsigned int)blue,
				 data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);
      /* update our knowledge of CGM's fill color */
      _plotter->cgm_fillcolor.red = red;
      _plotter->cgm_fillcolor.green = green;
      _plotter->cgm_fillcolor.blue = blue;
    }
}

void
#ifdef _HAVE_PROTOS
_c_set_bg_color(S___(Plotter *_plotter))
#else
_c_set_bg_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int red_long, green_long, blue_long;
  int red, green, blue;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->bgcolor.red;
  green_long = _plotter->drawstate->bgcolor.green;
  blue_long = _plotter->drawstate->bgcolor.blue;

  /* 24-bit or 48-bit RGB (as used in CGMs) */
  switch (CGM_BINARY_BYTES_PER_COLOR_COMPONENT)
    {
    case 1:
      /* 24-bit */
      red = (((unsigned int)red_long) >> 8) & 0xff;
      green = (((unsigned int)green_long) >> 8) & 0xff;
      blue = (((unsigned int)blue_long) >> 8) & 0xff;
      break;
    case 2:
    default:
      /* 48-bit */
      red = red_long;
      green = green_long;
      blue = blue_long;
      break;
    }

  /* update our knowledge of what CGM's background color should be (we'll
     use it only when we write the picture header) */
  _plotter->cgm_bgcolor.red = red;
  _plotter->cgm_bgcolor.green = green;
  _plotter->cgm_bgcolor.blue = blue;
}
