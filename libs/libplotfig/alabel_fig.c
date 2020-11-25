/* This file contains the device-specific versions of alabel(), called
   _alabel_standard() and _alabel_device().  This version is for
   libplotfig.

   The width of the string in user units is returned. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define FONT_TYPE_LATEX 0	/* don't use LaTeX fonts currently */
#define FONT_TYPE_PS 4

/* we print a warning if we print a label when the affine user->device
   coordinate map isn't just a uniform scaling plus a rotation and
   translation, since xfig doesn't support sheared or non-uniformly scaled
   fonts */
static Boolean _printed_font_scaling_warning = FALSE;

/* Fig justification types */
#define JUST_LEFT 0
#define JUST_CENTER 1
#define JUST_RIGHT 2

#define SCRIPTSIZE 0.6		/* rel. size of subscripts/superscripts */

#define SUBSCRIPT_DX 0.0
#define SUBSCRIPT_DY (-0.2)
#define SUPERSCRIPT_DX 0.0
#define SUPERSCRIPT_DY 0.375

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static Boolean _simple_string P__((const unsigned short *codestring));
static double _label_width_standard_codestring P__((const unsigned short *codestring));
static double _label_width_standard_string P__((const unsigned char *s, int master_font_index));
static void _alabel_standard_internal P__((const unsigned char *s, int h_just, int master_font_index));
#undef P__

/* _label_width_standard_string() computes the width (total delta x) of a
   character string to be rendered in one of the 35 standard fonts, in user
   units.  The string is just a string: a sequence of bytes (no font
   annotations, and no control codes).   The font is specified by an
   index into the master table in fontdb.c. */

static double
_label_width_standard_string (s, master_font_index)
     const unsigned char *s;
     int master_font_index;
{
  double user_font_size = _drawstate->fig_quantized_font_size;
  int index;
  int width = 0;
  unsigned char current_char;

  /* compute font index in master PS font table */
  for (index=0; s[index]!='\0'; index++)
    {
      current_char = (unsigned int)s[index];
      width 
	+= ((_ps_font_info[master_font_index]).width)[current_char];
    }

  /* multiply by font size (in user units), divided by 1000, and return */
  return user_font_size * (double)width / 1000.0;
}

/* _label_width_standard_codestring() computes the width (total delta x),
   in user units, of a character string to be rendered in the 35 standard
   fonts.  The string is really a codestring: it may include font
   annotations, and control codes.  `Font annotations' means that the
   characters can be selected from more than one font in a typeface. */

static double
_label_width_standard_codestring (codestring)
     const unsigned short *codestring;
{
  const unsigned short *cptr = codestring;
  double width = 0.0;
  double saved_font_size = _drawstate->font_size;
  double pushed_width = 0.0;	/* pushed by user */

  while (*cptr)			/* end when (unsigned short)0 is seen */
    {
      unsigned short c;

      c = *cptr;
      if (c & CONTROL_CODE)	
	{	
	  double user_font_size = _drawstate->fig_quantized_font_size;

	  /* parse control code */
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      width += SUBSCRIPT_DX * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _set_font_sizes();
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      width += SUPERSCRIPT_DX * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _set_font_sizes();
	      break;

	    case C_END_SUBSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _set_font_sizes();
	      width -= SUBSCRIPT_DX * user_font_size;
	      break;
	      
	    case C_END_SUPERSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _set_font_sizes();
	      width -= SUPERSCRIPT_DX * user_font_size;
	      break;
	      
	    case C_PUSH_LOCATION:
	      pushed_width = width;
	      break;
	      
	    case C_POP_LOCATION:
	      width = pushed_width;
	      break;
	      
	    case C_RIGHT_ONE_EM:
	      width += user_font_size;
	      break;
	      
	    case C_RIGHT_HALF_EM:
	      width += user_font_size / 2.0;
	      break;

	    case C_RIGHT_QUARTER_EM:
	      width += user_font_size / 4.0;
	      break;

	    case C_RIGHT_SIXTH_EM:
	      width += user_font_size / 6.0;
	      break;

	    case C_RIGHT_EIGHTH_EM:
	      width += user_font_size / 8.0;
	      break;

	      /* kludge: used for \rn macro only */
	    case C_RIGHT_RADICAL_SHIFT:
	      width += user_font_size * PS_RADICAL_WIDTH;
	      break;

	    case C_LEFT_ONE_EM:
	      width -= user_font_size;
	      break;
	      
	    case C_LEFT_HALF_EM:
	      width -= user_font_size / 2.0;
	      break;

	    case C_LEFT_QUARTER_EM:
	      width -= user_font_size / 4.0;
	      break;

	    case C_LEFT_SIXTH_EM:
	      width -= user_font_size / 6.0;
	      break;

	    case C_LEFT_EIGHTH_EM:
	      width -= user_font_size / 8.0;
	      break;

	      /* kludge: used for \rn macro only */
	    case C_LEFT_RADICAL_SHIFT:
	      width -= user_font_size * PS_RADICAL_WIDTH;
	      break;

	      /* unrecognized control code */
	    default:
	      break;
	    }
	}
      else			/* yow, an ordinary character */
	{
	  double user_font_size = _drawstate->fig_quantized_font_size;
	  unsigned char current_char;
	  int master_font_index;	/* index into master table */

	  /* extract font index from annotation */
	  master_font_index = (c >> FONT_SHIFT) & ONE_BYTE;

	  /* extract character, add its width */
	  current_char = c & ONE_BYTE; 
	  width += user_font_size 
	            * (((_ps_font_info[master_font_index]).width)[current_char] 
		       / 1000.0);
	}

      cptr++;			/* on to next element of codestring */
    }

  /* restore original font size */
  _drawstate->font_size = saved_font_size;
  _set_font_sizes();		/* restore other [fig-specific] fields */

  return width;
}

/* _labelwidth_standard() is just like _label_width_standard_codestring()
   above.  This is the version that is called when the user calls
   labelwidth(), if the current font is one of the 35 standard PS fonts. */

double
_labelwidth_standard (codestring)
     const unsigned short *codestring;
{
  double label_width;
  
  label_width = _label_width_standard_codestring (codestring);
  
  return label_width;
}

/* _alabel_standard_internal() outputs a single-font, single-font-size
   substring of a label in Fig format.  When this is called, the current
   point is on the intended baseline of the string.  Repositioning takes
   place after printing.  If string is left-justified, the motion is to the
   right; if it is right-justified, the motion is to the left.  If it is
   horizontally centered, there is no repositioning.  */

static void
_alabel_standard_internal (s, h_just, master_font_index)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
     int master_font_index;
{
  int len;
  unsigned char *ptr, *t;
  double theta, costheta, sintheta;
  double label_width, label_ascent;
  double dx, translation_x, translation_y;
  double initial_x, initial_y;
  double horizontal_x, horizontal_y, vertical_x, vertical_y;
  double horizontal_fig_length, vertical_fig_length;
  double horizontal_fig_x, vertical_fig_x;
  double horizontal_fig_y, vertical_fig_y;
  double angle_device;
  
  if (*s == (unsigned char)'\0')
    return;

  /* label rotation angle in radians */
  theta = M_PI * _drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* compute label height and width, in user units */
  label_width = _label_width_standard_string (s, master_font_index);
  label_ascent  = _drawstate->fig_quantized_font_size * (_ps_font_info[master_font_index]).font_ascent / 1000.0;
  
  /* vector along which we'll move when we print the label (user frame) */
  switch (h_just)
    {
    case JUST_LEFT:
    default:
      dx = label_width;
      break;
    case JUST_CENTER:
      dx = 0.0;
      break;
    case JUST_RIGHT:
      dx = - label_width;
      break;
    }

  translation_x = costheta * dx;
  translation_y = sintheta * dx;

  /* vector along baseline of label, and an orthogonal vector which is the
     other side of a rectangle containing the portion of the string above
     the baseline (both in the user frame) */

  horizontal_x = costheta * label_width;
  horizontal_y = sintheta * label_width;

  vertical_x =  - sintheta * label_ascent;
  vertical_y =    costheta * label_ascent;
  
  /* Convert two above orthogonal vectors to the device frame, and compute
     their lengths.  In the device frame they may no longer be orthogonal.
     But xfig supports setting up only rectangular `hot regions', so we'll
     use their lengths as the sides of a rectangle.  If user coor->device
     coor map is highly sheared, this would be inappropriate. 

     Incidentally, the height of the rectangular hot region should really
     be the string ascent (from its bounding box), not the font ascent.
     But since we don't include the bounding boxes of individual characters
     in our fontdb.c, we have no way of computing the former. */

  horizontal_fig_x = XDV(horizontal_x, horizontal_y);
  horizontal_fig_y = YDV(horizontal_x, horizontal_y);  
  horizontal_fig_length = sqrt(horizontal_fig_x * horizontal_fig_x
				+ horizontal_fig_y * horizontal_fig_y);

  /* text angle in device frame (note flipped-y convention) */
  angle_device = - _xatan2 (horizontal_fig_y, horizontal_fig_x);
  if (angle_device == 0.0)
    angle_device = 0.0;		/* remove sign bit if any */
  
  vertical_fig_x = XDV(vertical_x, vertical_y);
  vertical_fig_y = YDV(vertical_x, vertical_y);  
  vertical_fig_length = sqrt(vertical_fig_x * vertical_fig_x
				+ vertical_fig_y * vertical_fig_y);
  
  /* where we should start from, in device frame (i.e. in Fig units) */
  initial_x = XD((_drawstate->pos).x, (_drawstate->pos).y);
  initial_y = YD((_drawstate->pos).x, (_drawstate->pos).y);

  /* evaluate fig colors lazily, i.e. only when needed */
  _evaluate_fig_fgcolor();
  
  /* escape all backslashes in the text string, before output */
  len = strlen ((char *)s);
  ptr = (unsigned char *)_plot_xmalloc ((unsigned int)(4 * len + 1));
  t = ptr;
  while (*s)
    {
      switch (*s)
	{
	case '\\':
	  *ptr++ = (unsigned char)'\\';
	  *ptr++ = *s++;
          break;
	default:
          if GOOD_PRINTABLE_ASCII (*s)
	    *ptr++ = *s++;
          else
            {	    
               sprintf ((char *)ptr, "\\%03o", (unsigned int)*s);
               ptr += 4;
               s++;
            }
          break;
	}
    }
  *ptr = (unsigned char)'\0';

  /* warn luser of the horrible thing we're doing, if any */
  if (!_drawstate->transform.uniform && !_printed_font_scaling_warning)
    {
      fprintf (stderr, "libplotfig: no support for obliquing of fonts\n");
      _printed_font_scaling_warning = TRUE;
    }
      
  /* recompute xfig's `depth' attribute, and keep track of the drawing
     priority of the object we're drawing */
  if (_drawstate->fig_last_priority > TEXT_PRIORITY)
    if (_drawstate->fig_drawing_depth > 0)
      (_drawstate->fig_drawing_depth)--;
  _drawstate->fig_last_priority = TEXT_PRIORITY;

  sprintf(_outbuf.current,
	  "#TEXT\n%d %d %d %d %d %d %.3f %.3f %d %.3f %.3f %d %d %s\\001\n",
	  4,			/* text object */
	  /* xfig supports 3 justification types: left, center, or right. */
	  h_just,		/* horizontal justification type */
	  _drawstate->fig_fgcolor, /* pen color */
	  _drawstate->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _ps_font_info[master_font_index].fig_id, /* Fig font id */
	  (double)_drawstate->fig_font_point_size, /* point size (float) */
	  angle_device,		/* text rotation in radians (float) */
	  FONT_TYPE_PS,		/* Fig font type */
	  /* these next two are used only for setting up `hot spots' */
	  vertical_fig_length, /* string height, Fig units (float) */
	  horizontal_fig_length, /* string width, Fig units (float) */
	  /* coors of origin of label, in Fig units */
	  IROUND(initial_x), 
	  IROUND(initial_y),
	  t);			/* munged string */
  free (t);
  _update_buffer (&_outbuf);

  /* reposition after printing substring */
  (_drawstate->pos).x += translation_x;
  (_drawstate->pos).y += translation_y;
 
  return;
}

/* test whether a controlified string is simple in the sense that it
   consists of characters in the same font, and no control codes */
static
Boolean
_simple_string (codestring)
     const unsigned short *codestring;
{
  const unsigned short *cptr = codestring;
  unsigned short c, d;
  int font_index;

  if (*codestring == 0)
    return TRUE;
  c = *codestring;
   if (c & CONTROL_CODE)
    return FALSE;
  font_index = (c >> FONT_SHIFT) & ONE_BYTE;
  while ((d = *cptr++) != 0)
    {
      int local_font_index;

      if (d & CONTROL_CODE)
	return FALSE;
      local_font_index = (d >> FONT_SHIFT) & ONE_BYTE;      
      if (local_font_index != font_index)
	return FALSE;
    }
  return TRUE;
}

double
_alabel_standard (x_justify, y_justify, codestring)
     int x_justify, y_justify;
     const unsigned short *codestring;
{
  char x_justify_c, y_justify_c;
  double x_offset, y_offset;
  double theta, costheta, sintheta;
  double userdx, userdy;
  double x_displacement;
  double ascent, descent;
  const unsigned short *cptr = codestring;
  double overall_width;
  double initial_font_size = _drawstate->font_size;
  double initial_user_font_size = _drawstate->fig_quantized_font_size;
  double initial_position_x = (_drawstate->pos).x;
  double initial_position_y = (_drawstate->pos).y;
  double pushed_position_x = (_drawstate->pos).x; /* user can push */
  double pushed_position_y = (_drawstate->pos).y;
  int master_font_index;	/* index into master table */
  int h_just;
  Boolean simple_string;
  
  /* Compute index of current font, in master PS font table.  We use
     this only for vertical justification.  This is inelegant: we should
     really scan the codestring to determine the font in which the first
     character should be rendered, and use its index instead.  Or something
     like that. */
  master_font_index =
     (_ps_typeface_info[_drawstate->typeface_index].fonts)[_drawstate->font_index];
     
  /* now figure out the offsets due to justification */
  x_justify_c = (char)x_justify;
  y_justify_c = (char)y_justify;  

  switch (x_justify_c)
    {
    case 'l': /* left justified */
    default:
      x_offset = 0.0;
      x_displacement = 1.0;
      /* range [0,1] */
      h_just = JUST_LEFT;
      break;

    case 'c': /* centered */
      x_offset = -0.5;
      x_displacement = 0.0;
      /* range [-0.5,0.5] */
      h_just = JUST_CENTER;
      break;

    case 'r': /* right justified */
      x_offset = -1.0;
      x_displacement = -1.0;
      /* range [-1,0] */
      h_just = JUST_RIGHT;
      break;
    }

  /* If codestring is a string in a single font, with no control codes,
     we'll output it using native xfig justification, rather than
     positioning a left-justified string by hand.  In other words if right
     or centered justification was specified when alabel() was called, the
     string in the xfig drawing will have the same justification.  Anything
     else would exasperate the user, even if the positioning is correct. */
  simple_string = _simple_string (codestring);
  if (simple_string)
    x_offset = 0.0;
  else
    h_just = JUST_LEFT;		/* will use x_offset to position by hand */

  /* need these to compute offset for vertical justification */
  ascent = (double)((_ps_font_info[master_font_index]).font_ascent);
  descent = (double)((_ps_font_info[master_font_index]).font_descent);

  switch (y_justify_c)
    {
    case 'b':			/* current point is at bottom */
      y_offset =  descent / 1000.0;
      break;

    case 'x':			/* current point is on baseline */
    default:
      y_offset =  0.0;
      break;

    case 'c':		/* current point is midway between bottom, top */
      y_offset =  (descent - ascent) / 2000.0;
      break;

    case 't':			/* current point is at top */
      y_offset = - ascent / 1000.0;
      break;
    }

  /* label width, in user units */
  overall_width = _label_width_standard_codestring (codestring);
     
  /* justification-related offsets we'll carry out, in user units */
  userdx = x_offset * overall_width;
  userdy = y_offset * initial_user_font_size;
  
  /* label rotation angle in radians */
  theta = M_PI * _drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* perform offset, in user units; after this, current point will be on
     intended baseline of label */
  (_drawstate->pos).x += costheta * userdx - sintheta * userdy;
  (_drawstate->pos).y += sintheta * userdx + costheta * userdy;

  /* now loop through codestring, processing control codes and extracting
     single-font substrings; _alabel_standard_internal() is called on each */
  while (*cptr)			/* end when NULL is seen */
    {
      unsigned short c;

      c = *cptr;
      if (c & CONTROL_CODE)
	{
	  double user_font_size = _drawstate->fig_quantized_font_size;

	  /* parse control code */
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      (_drawstate->pos).x += (costheta * SUBSCRIPT_DX 
				      - sintheta * SUBSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y += (sintheta * SUBSCRIPT_DX
				      + costheta * SUBSCRIPT_DY) * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _set_font_sizes(); /* update fig_quantized_font_size field */
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      (_drawstate->pos).x += (costheta * SUPERSCRIPT_DX 
				      - sintheta * SUPERSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y += (sintheta * SUPERSCRIPT_DX
				      + costheta * SUPERSCRIPT_DY) * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _set_font_sizes(); /* update fig_quantized_font_size field */
	      break;

	    case C_END_SUBSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _set_font_sizes(); /* update fig_quantized_font_size field */
	      user_font_size = _drawstate->fig_quantized_font_size;
	      (_drawstate->pos).x -= (costheta * SUBSCRIPT_DX 
				      - sintheta * SUBSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y -= (sintheta * SUBSCRIPT_DX
				      + costheta * SUBSCRIPT_DY) * user_font_size;
	      break;

	    case C_END_SUPERSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _set_font_sizes(); /* update fig_quantized_font_size field */
	      user_font_size = _drawstate->fig_quantized_font_size;
	      (_drawstate->pos).x -= (costheta * SUPERSCRIPT_DX 
				      - sintheta * SUPERSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y -= (sintheta * SUPERSCRIPT_DX
				      + costheta * SUPERSCRIPT_DY) * user_font_size;
	      break;
	      
	    case C_PUSH_LOCATION:
	      pushed_position_x = (_drawstate->pos).x;
	      pushed_position_y = (_drawstate->pos).y;
	      break;
	      
	    case C_POP_LOCATION:
	      (_drawstate->pos).x = pushed_position_x;
	      (_drawstate->pos).y = pushed_position_y;
	      break;
	      
	    case C_RIGHT_ONE_EM:
	      (_drawstate->pos).x += costheta * user_font_size;
	      (_drawstate->pos).y += sintheta * user_font_size;
	      break;
	      
	    case C_RIGHT_HALF_EM:
	      (_drawstate->pos).x += costheta * user_font_size / 2.0;
	      (_drawstate->pos).y += sintheta * user_font_size / 2.0;
	      break;

	    case C_RIGHT_QUARTER_EM:
	      (_drawstate->pos).x += costheta * user_font_size / 4.0;
	      (_drawstate->pos).y += sintheta * user_font_size / 4.0;
	      break;

	    case C_RIGHT_SIXTH_EM:
	      (_drawstate->pos).x += costheta * user_font_size / 6.0;
	      (_drawstate->pos).y += sintheta * user_font_size / 6.0;
	      break;

	    case C_RIGHT_EIGHTH_EM:
	      (_drawstate->pos).x += costheta * user_font_size / 8.0;
	      (_drawstate->pos).y += sintheta * user_font_size / 8.0;
	      break;

	      /* kludge: used for \rn macro only */
	    case C_RIGHT_RADICAL_SHIFT:
	      (_drawstate->pos).x += costheta * user_font_size * PS_RADICAL_WIDTH;
	      (_drawstate->pos).y += sintheta * user_font_size * PS_RADICAL_WIDTH;
	      break;

	    case C_LEFT_ONE_EM:
	      (_drawstate->pos).x -= costheta * user_font_size;
	      (_drawstate->pos).y -= sintheta * user_font_size;
	      break;
	      
	    case C_LEFT_HALF_EM:
	      (_drawstate->pos).x -= costheta * user_font_size / 2.0;
	      (_drawstate->pos).y -= sintheta * user_font_size / 2.0;
	      break;

	    case C_LEFT_QUARTER_EM:
	      (_drawstate->pos).x -= costheta * user_font_size / 4.0;
	      (_drawstate->pos).y -= sintheta * user_font_size / 4.0;
	      break;

	    case C_LEFT_SIXTH_EM:
	      (_drawstate->pos).x -= costheta * user_font_size / 6.0;
	      (_drawstate->pos).y -= sintheta * user_font_size / 6.0;
	      break;

	    case C_LEFT_EIGHTH_EM:
	      (_drawstate->pos).x -= costheta * user_font_size / 8.0;
	      (_drawstate->pos).y -= sintheta * user_font_size / 8.0;
	      break;

	      /* kludge: used for \rn macro only */
	    case C_LEFT_RADICAL_SHIFT:
	      (_drawstate->pos).x -= costheta * user_font_size * PS_RADICAL_WIDTH;
	      (_drawstate->pos).y -= sintheta * user_font_size * PS_RADICAL_WIDTH;
	      break;

	      /* unrecognized control code */
	    default:
	      break;
	    }

	  cptr++;		/* on to next element of codestring */
	}

      else		/* an ordinary char, with font annotation */
	{
	  unsigned char *s, *sptr;

	  /* extract substring consisting of characters in the same font */
	  master_font_index = (c >> FONT_SHIFT) & ONE_BYTE;
	  sptr = s 
	    = (unsigned char *)_plot_xmalloc ((unsigned int)(4 * _codestring_len (cptr) + 1));
	  while (*cptr 
		 && (*cptr & CONTROL_CODE) == 0 
		 && ((*cptr >> FONT_SHIFT) & ONE_BYTE) == master_font_index)
	    *sptr++ = (*cptr++) & ONE_BYTE;
	  *sptr = (unsigned char)'\0';

	  /* plot the substring, and free it */
	  _alabel_standard_internal (s, h_just, master_font_index);
	  free (s);
	}
    }

  /* restore original font size */
  _drawstate->font_size = initial_font_size;
  _set_font_sizes();		/* restore other [fig-specific] fields */

  /* restore position to what it was before printing label */
  (_drawstate->pos).x = initial_position_x;
  (_drawstate->pos).y = initial_position_y;

#if 1				/* 1 will become WIDELABEL */
  /* shift after printing label, if desired (user units) */
  (_drawstate->pos).x += costheta * x_displacement * overall_width;
  (_drawstate->pos).y += sintheta * x_displacement * overall_width;
#endif

  /* return length of string in user units */
  return overall_width;
}

/* we don't support the device-specific fonts (i.e. LaTeX fonts) */
double
_alabel_device (x_justify, y_justify, codestring)
     int x_justify, y_justify;
     const unsigned short *codestring;
{
  return 0.0;
}

double
_labelwidth_device (codestring)
     const unsigned short *codestring;
{
  return 0.0;
}
