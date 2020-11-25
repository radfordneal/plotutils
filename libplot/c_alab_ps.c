#include "sys-defines.h"
#include "extern.h"

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

/* CGM horizontal alignment styles, indexed by internal number
   (left/center/right) */
static const int _cgm_horizontal_alignment_style[] =
{ CGM_ALIGN_LEFT, CGM_ALIGN_CENTER, CGM_ALIGN_RIGHT };

/* corresponding strings, as used in the text encoding */
static const char * _cgm_horizontal_alignment_style_string[] =
{ "left", "ctr", "right" };

/* CGM vertical alignment styles, indexed by internal number
   (top/half/base/bottom) */
static const int _cgm_vertical_alignment_style[] =
{ CGM_ALIGN_TOP, CGM_ALIGN_HALF, CGM_ALIGN_BASE, CGM_ALIGN_BOTTOM };

/* corresponding strings, as used in the text encoding */
static const char * _cgm_vertical_alignment_style_string[] =
{ "top", "half", "base", "bottom" };

/* This prints a single-font, single-font-size label, and repositions to
   the end after printing. */

double
#ifdef _HAVE_PROTOS
_c_falabel_ps (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_c_falabel_ps (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
     int v_just;  /* vertical justification: JUST_TOP, HALF, BASE, BOTTOM */
#endif
{
  int master_font_index, desired_cgm_font_id;
  double theta, costheta, sintheta;
  double user_text_transformation_matrix[6];
  double text_transformation_matrix[6];
  int desired_char_base_vector_x, desired_char_base_vector_y;
  int desired_char_up_vector_x, desired_char_up_vector_y;
  double relative_cap_height, user_cap_height;
  double up_vector_x, up_vector_y, cap_height;
  double base_vector_x, base_vector_y, base_width;
  int desired_char_height, desired_base_width;
  bool font_is_symbol, need_lower_half, need_upper_half;
  bool set_lower_half_charset = false, set_upper_half_charset = false;
  int lower_half_charset = 0, upper_half_charset = 0; /* dummy values */
  const unsigned char *t;
  double width, x_displacement;
  int desired_cgm_h_alignment, desired_cgm_v_alignment;
  int byte_count, data_byte_count, data_len;

  /* sanity check */
  if (_plotter->drawstate->font_type != F_POSTSCRIPT)
    return 0.0;

  /* if empty string, nothing to do */
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* set CGM text color (pass hint) */
  _plotter->drawstate->cgm_object_type = CGM_OBJECT_TEXT;
  _plotter->set_pen_color (S___(_plotter));

  /* compute index of font in master table of PS fonts, in g_fontdb.c */
  master_font_index =
    (_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* flag current PS font as used on this page */
  _plotter->page->ps_font_used[master_font_index] = true;

  /* synchronize CGM's font with our notion of current font, by setting the
     CGM font index (in range 1..35; as explained in g_fontdb.c, the
     traditional `Adobe 13' will be placed in slots 1..13) */
  desired_cgm_font_id = _ps_font_to_cgm_font_id[master_font_index];
  if (_plotter->cgm_font_id != desired_cgm_font_id)
    /* emit "TEXT FONT INDEX" command */
    {
      data_len = 2;		/* 2 bytes per index */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 10,
				data_len, &byte_count,
				"TEXTFONTINDEX");
      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
		       /* index value in range 1..35, not 0..34 */
		       desired_cgm_font_id + 1,
		       data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);
      /* update CGM font id */
      _plotter->cgm_font_id = desired_cgm_font_id;
    }

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* this transformation matrix rotates, and translates: it maps (0,0) to
     the origin of the string, in user coordinates */
  user_text_transformation_matrix[0] = costheta;
  user_text_transformation_matrix[1] = sintheta;
  user_text_transformation_matrix[2] = - sintheta;
  user_text_transformation_matrix[3] = costheta;
  user_text_transformation_matrix[4] = _plotter->drawstate->pos.x;
  user_text_transformation_matrix[5] = _plotter->drawstate->pos.y;

  /* Construct a temporary matrix that rotates, translates, and then maps
     to device coordinates.  This matrix transforms from a frame in which
     nominal character sizes are roughly 1 unit in the horizontal and
     vertical directions, to device coordinates. */
  _matrix_product (user_text_transformation_matrix, 
		   _plotter->drawstate->transform.m,
		   text_transformation_matrix);

  /* Character base and up vectors, in the device frame, are what vectors
     (1,0) and (0,1) in the user frame are mapped to.  According to CGM
     documentation, their overall normalization isn't important, but the
     ratio between their lengths affects the aspect ratio of the font.  The
     `4000' factor is arbitrary; but it gives good quantization to integers. */
  {
    double base_x = text_transformation_matrix[0];
    double base_y = text_transformation_matrix[1];
    double up_x = text_transformation_matrix[2];
    double up_y = text_transformation_matrix[3];
    double base_len = sqrt (base_x * base_x + base_y * base_y);
    double up_len = sqrt (up_x * up_x + up_y * up_y);
    double max_len = DMAX(base_len, up_len);
    
    if (max_len != 0.0)
      {
	base_x /= max_len;
	base_y /= max_len;
	up_x /= max_len;
	up_y /= max_len;
      }

#define QUANTIZATION_FACTOR 4000
    desired_char_base_vector_x = IROUND(QUANTIZATION_FACTOR * base_x);
    desired_char_base_vector_y = IROUND(QUANTIZATION_FACTOR * base_y);
    desired_char_up_vector_x = IROUND(QUANTIZATION_FACTOR * up_x);
    desired_char_up_vector_y = IROUND(QUANTIZATION_FACTOR * up_y);
  }
  
  if (_plotter->cgm_char_base_vector_x != desired_char_base_vector_x
      || _plotter->cgm_char_base_vector_y != desired_char_base_vector_y
      || _plotter->cgm_char_up_vector_x != desired_char_up_vector_x
      || _plotter->cgm_char_up_vector_y != desired_char_up_vector_y)
    /* emit CHARACTER ORIENTATION command */
    {
      data_len = 4 * CGM_BINARY_BYTES_PER_INTEGER; /* args: 4 integers */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 16,
				data_len, &byte_count,
				"CHARORI");
      _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			 desired_char_up_vector_x,
			 data_len, &data_byte_count, &byte_count);
      _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			 desired_char_up_vector_y,
			 data_len, &data_byte_count, &byte_count);
      _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			 desired_char_base_vector_x,
			 data_len, &data_byte_count, &byte_count);
      _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			 desired_char_base_vector_y,
			 data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);
    
      /* update char and base vectors */
      _plotter->cgm_char_base_vector_x = desired_char_base_vector_x;
      _plotter->cgm_char_base_vector_y = desired_char_base_vector_y;
      _plotter->cgm_char_up_vector_x = desired_char_up_vector_x;
      _plotter->cgm_char_up_vector_y = desired_char_up_vector_y;
    }

  /* cap height as fraction of font height */
  relative_cap_height = 
    _ps_font_info[master_font_index].font_cap_height / 1000.0;
  /* cap height in user frame */
  user_cap_height = relative_cap_height * _plotter->drawstate->true_font_size;
  /* true up vector (pointing up to cap height level) in device frame */
  up_vector_x = user_cap_height * text_transformation_matrix[2];
  up_vector_y = user_cap_height * text_transformation_matrix[3];
  /* length of up vector in device frame, i.e. desired cap height */
  cap_height = sqrt (up_vector_x * up_vector_x + up_vector_y * up_vector_y);
  /* in CGM, `character height' means cap height, measured along up vector */
  desired_char_height = IROUND(cap_height);
  
  if (_plotter->cgm_char_height != desired_char_height)
    /* emit CHARACTER HEIGHT command */
    {
      data_len = CGM_BINARY_BYTES_PER_INTEGER; /* args: 1 integer */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 15,
				data_len, &byte_count,
				"CHARHEIGHT");
      _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			 desired_char_height,
			 data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update character height */
      _plotter->cgm_char_height = desired_char_height;
    }

  /* is label to be rendered in Symbol font (which has its own charsets)? */
  if (strcmp (_ps_font_info[master_font_index].ps_name, "Symbol") == 0)
    font_is_symbol = true;
  else
    font_is_symbol = false;

  /* will we be using lower, upper halves of font? */
  need_lower_half = need_upper_half = false;
  for (t = s; *t != (unsigned char)'\0'; t++)
    {
      if ((*t) <= 127)
	need_lower_half = true;
      else
	need_upper_half = true;
    }

  /* Do we need to switch lower/upper CGM charsets?  Our indexing of
     charsets (see c_defplot.c) is: 1=lower half ISO-Latin-1, 2=upper half
     ISO-Latin-1, 3=lower half Symbol, 4=upper half Symbol. */

  if (font_is_symbol)
    /* may need to switch to CGM's Symbol charset(s) */
    {
      if (need_lower_half && _plotter->cgm_charset_lower != 3)
	{
	  set_lower_half_charset = true;
	  lower_half_charset = 3;
	}
      if (need_upper_half && _plotter->cgm_charset_upper != 4)
	{
	  set_upper_half_charset = true;
	  upper_half_charset = 4;
	}
    }
  else
    /* font is ISO-Latin-1, for CGM purposes; may need to switch charset(s) */
    {
      if (need_lower_half && _plotter->cgm_charset_lower != 1)
	{
	  set_lower_half_charset = true;
	  lower_half_charset = 1;
	}
      if (need_upper_half && _plotter->cgm_charset_upper != 2)
	{
	  set_upper_half_charset = true;
	  upper_half_charset = 2;
	}
    }

  if (set_lower_half_charset)
    /* emit SET CHARACTER SET INDEX command */
    {
      data_len = 2;		/* 2 bytes per index */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 19,
				data_len, &byte_count,
				"CHARSETINDEX");
      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
		       lower_half_charset,
		       data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update our knowledge of CGM's lower-half charset */
      _plotter->cgm_charset_lower = lower_half_charset;
    }

  if (set_upper_half_charset)
    /* emit SET ALTERNATE CHARACTER SET INDEX command */
    {
      data_len = 2;		/* 2 bytes per index */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 20,
				data_len, &byte_count,
				"ALTCHARSETINDEX");
      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
		       upper_half_charset,
		       data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update our knowledge of CGM's upper-half charset */
      _plotter->cgm_charset_upper = upper_half_charset;
    }

  /* update CGM text alignment if necessary */
  desired_cgm_h_alignment = _cgm_horizontal_alignment_style[h_just];
  desired_cgm_v_alignment = _cgm_vertical_alignment_style[v_just];
  if (_plotter->cgm_horizontal_text_alignment != desired_cgm_h_alignment
      || _plotter->cgm_vertical_text_alignment != desired_cgm_v_alignment)
    /* emit "TEXT ALIGNMENT" command (args = 2 enums, 2 reals) */
    {
      const char *desired_cgm_h_alignment_string, *desired_cgm_v_alignment_string;

      desired_cgm_h_alignment_string = 
	_cgm_horizontal_alignment_style_string[h_just];
      desired_cgm_v_alignment_string = 
	_cgm_vertical_alignment_style_string[v_just];

      data_len = 2 * 2 + 2 * 4;	/* 2 bytes per enum, 4 bytes per real */
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 18,
				data_len, &byte_count,
				"TEXTALIGN");
      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
		      desired_cgm_h_alignment,
		      data_len, &data_byte_count, &byte_count,
		      desired_cgm_h_alignment_string);
      _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
		      desired_cgm_v_alignment,
		      data_len, &data_byte_count, &byte_count,
		      desired_cgm_v_alignment_string);
      _cgm_emit_real_fixed_point (_plotter->page, false, _plotter->cgm_encoding,
				  0.0,
				  data_len, &data_byte_count, &byte_count);
      _cgm_emit_real_fixed_point (_plotter->page, false, _plotter->cgm_encoding,
				  0.0,
				  data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				    &byte_count);

      /* update our knowledge of text alignment */
      _plotter->cgm_horizontal_text_alignment = desired_cgm_h_alignment;
      _plotter->cgm_vertical_text_alignment = desired_cgm_v_alignment;
    }

  if (_plotter->cgm_max_version >= 3)
    /* can specify type of restricted text */
    {
      if (_plotter->cgm_restricted_text_type 
	  != CGM_RESTRICTED_TEXT_TYPE_BOXED_CAP)
	/* emit "RESTRICTED TEXT TYPE" command */
	{
	  data_len = 2;	/* 2 bytes per index */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 42,
				    data_len, &byte_count,
				    "RESTRTEXTTYPE");
	  _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			   CGM_RESTRICTED_TEXT_TYPE_BOXED_CAP,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);

	  /* update restricted text type, and CGM version for this page */
	  _plotter->cgm_restricted_text_type = 
	    CGM_RESTRICTED_TEXT_TYPE_BOXED_CAP;
	  _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	}
    }

  /* compute string width in user coordinates */
  width = _plotter->flabelwidth_ps (R___(_plotter) s);

  /* true base vector (pointing across to end of string) in device frame */
  base_vector_x = width * text_transformation_matrix[0];
  base_vector_y = width * text_transformation_matrix[1];
  /* length of base vector in device frame, i.e. width of string */
  base_width = sqrt (base_vector_x * base_vector_x + base_vector_y * base_vector_y);
  /* same (integer device coordinates) */
  desired_base_width = IROUND(base_width);

  /* emit "RESTRICTED TEXT" command (args= 2 ints, 1 point, 1 enum, 1 string)*/
  {
    int string_length, encoded_string_length;
    double xdev, ydev;
    int xdev_int, ydev_int;

    string_length = strlen ((const char *)s);
    encoded_string_length = CGM_BINARY_BYTES_PER_STRING(string_length);
    /* bytes per integer coordinate may vary; 2 bytes per enum */
    data_len = (4 * CGM_BINARY_BYTES_PER_INTEGER) + 2 + encoded_string_length;
    byte_count = data_byte_count = 0;

    xdev = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
    ydev = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
    xdev_int = IROUND(xdev);
    ydev_int = IROUND(ydev);

    _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
			      CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 5,
			      data_len, &byte_count,
			      "RESTRTEXT");
    _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
		       desired_base_width,
		       data_len, &data_byte_count, &byte_count);
    _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
		       desired_char_height,
		       data_len, &data_byte_count, &byte_count);
    _cgm_emit_point (_plotter->page, false, _plotter->cgm_encoding,
		     xdev_int, ydev_int,
		     data_len, &data_byte_count, &byte_count);
    _cgm_emit_enum (_plotter->page, false, _plotter->cgm_encoding,
		    1,
		    data_len, &data_byte_count, &byte_count,
		    "final");
    _cgm_emit_string (_plotter->page, false, _plotter->cgm_encoding,
		      (const char *)s,
		      string_length, true,
		      data_len, &data_byte_count, &byte_count);
    _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
				  &byte_count);

    /* update CGM profile for this page */
    if (string_length > 254)
      _plotter->cgm_page_profile = IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
  }

  /* reposition after printing string */
  switch (h_just)
    {
    case JUST_LEFT:
    default:
      x_displacement = 1.0;
      break;
    case JUST_CENTER:
      x_displacement = 0.0;
      break;
    case JUST_RIGHT:
      x_displacement = -1.0;
      break;
    }
  _plotter->drawstate->pos.x += costheta * x_displacement * width;
  _plotter->drawstate->pos.y += sintheta * x_displacement * width;

  /* return string width in user coordinates */
  return width;
}
