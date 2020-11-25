/* This file contains the device-specific versions of alabel(), called
   _alabel_standard() and _alabel_device().  This version is for libplotX.

   The width of the string in user units is returned. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define SCRIPTSIZE 0.6		/* rel. size of subscripts/superscripts */

#define SUBSCRIPT_DX 0.0
#define SUBSCRIPT_DY (-0.2)
#define SUPERSCRIPT_DX 0.0
#define SUPERSCRIPT_DY 0.375

/* font we use for symbol escapes if the current font is a user-specified
   one that doesn't belong to any of our builtin typefaces */
#define X_SYMBOL_FONT "symbol-medium-r-normal"

/* forward reference */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static double _label_width_standard_codestring P__((const unsigned short *codestring));
static double _label_width_standard_string P__((const unsigned char *s));
static int _raw_string_width P__((const unsigned char *s));
static void _alabel_standard_internal P__((const unsigned char *s));
#undef P__

/* _raw_string_width() computes what the width of a label would be, in
   pixels (i.e. device coordinates), if a non-rotated, non-sheared
   1000-pixel version of the font is used.  It must be divided by 1000, and
   multiplied by the font size in user units, to yield the string width in
   user units.  The function label_width_standard_string() does this.

   The string is just a string: a sequence of bytes (no font annotations,
   and no control codes). */

static int
_raw_string_width (s)
     const unsigned char *s;
{
  int offset = 0;
  
  if (_drawstate->native_positioning)
    /* have a non-XLFD font, or an XLFD with zero textrotation, no shearing */
    offset = IROUND(1000.0 * XTextWidth (_drawstate->x_font_struct, (char *)s, (int)(strlen((char *)s))) / _drawstate->font_pixmatrix[0]);
  else				
    /* necessarily have an XLFD font */
    {
      while (*s)
	{
	  int charno = *s;
	  int char_metric_offset = 
	    charno - _drawstate->x_font_struct->min_char_or_byte2;
	  
	  offset += (_drawstate->x_font_struct->per_char ?
		     _drawstate->x_font_struct->per_char[char_metric_offset].attributes :
		     _drawstate->x_font_struct->min_bounds.attributes);
	  s++;
	}
    }

  return offset;
}

static double
_label_width_standard_string (s)
     const unsigned char *s;
{
  double label_width;
  double user_font_size = _drawstate->x_font_size;
  
  label_width = user_font_size * (double)_raw_string_width (s) / 1000.0;
  
  return label_width;
}

/* _label_width_standard_codestring() computes the width (total delta x),
   in user units, of a character string to be rendered in the 35 standard
   fonts.  The string is really a codestring: it may include font
   annotations, and control codes.  `Font annotations' means that the
   characters can be selected from more than one font in the currently used
   typeface.

   This routine also be called if the current font is not one of the
   builtin fonts, but is some other (user-specified) X font.  In that case
   the font annotations will simply indicate whether or not a symbol font
   should be switched to, for the purpose of symbol escapes.). */

static double
_label_width_standard_codestring (codestring)
     const unsigned short *codestring;
{
  const unsigned short *cptr = codestring;
  double width = 0.0;
  double initial_font_size = _drawstate->font_size;
  char *initial_font_name;
  double pushed_width = 0.0;	/* pushed by user */
  Boolean changed_font = FALSE;
  int master_font_index;
  int initial_font_type = _drawstate->font_type;

  /* determine index of font, for comparison with font annotations in string */
  switch (_drawstate->font_type)
    {
    case F_STANDARD:
    default:
      master_font_index =
	(_ps_typeface_info[_drawstate->typeface_index].fonts)[_drawstate->font_index];
      break;
    case F_DEVICE_SPECIFIC:
      master_font_index = 1;	/* `1' just means the font we start out with */
      break;
    }

  /* save font name */
  initial_font_name = (char *)_plot_xmalloc (1 + strlen (_drawstate->font_name));
  strcpy (initial_font_name, _drawstate->font_name);

  while (*cptr)			/* end when (unsigned short)0 is seen */
    {
      unsigned short c;

      c = *cptr;
      if (c & CONTROL_CODE)	
	{	
	  double user_font_size = _drawstate->x_font_size;

	  /* parse control code */
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      width += SUBSCRIPT_DX * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      width += SUPERSCRIPT_DX * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      break;

	    case C_END_SUBSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      width -= SUBSCRIPT_DX * user_font_size;
	      break;
	      
	    case C_END_SUPERSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
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

	  cptr++;		/* on to next element of codestring */
	}

      else		/* an ordinary character, with font annotation */
	{
	  unsigned char *s, *sptr;
	  int new_master_font_index = (c >> FONT_SHIFT) & ONE_BYTE;

	  /* perform font switching if requested */
	  if (new_master_font_index != master_font_index)
	    {
	      free (_drawstate->font_name);
	      switch (_drawstate->font_type)
		{
		case F_STANDARD:
		default:
		  _drawstate->font_name =
		    (char *)_plot_xmalloc(1 + strlen (_ps_font_info[new_master_font_index].x_name));
		  strcpy (_drawstate->font_name, _ps_font_info[new_master_font_index].x_name);
		  break;
		case F_DEVICE_SPECIFIC:
		  if (new_master_font_index == 0) /* symbol font */
		    {
		      _drawstate->font_name =
			(char *)_plot_xmalloc(1 + strlen (X_SYMBOL_FONT));
		      strcpy (_drawstate->font_name, X_SYMBOL_FONT);
		    }
		  else		/* 1, i.e. restore font we started out with */
		    {
		      _drawstate->font_name =
			(char *)_plot_xmalloc(1 + strlen (initial_font_name));
		      strcpy (_drawstate->font_name, initial_font_name);
		    }
		  break;
		}
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      master_font_index = new_master_font_index;
	      changed_font = TRUE;
	    }
	  
	  /* extract substring consisting of characters in the same font */
	  sptr = s 
	    = (unsigned char *)_plot_xmalloc ((unsigned int) (4 * _codestring_len (cptr) + 1));
	  while (*cptr 
		 && (*cptr & CONTROL_CODE) == 0 
		 && ((*cptr >> FONT_SHIFT) & ONE_BYTE) == master_font_index)
	    *sptr++ = (*cptr++) & ONE_BYTE;
	  *sptr = (unsigned char)'\0';

	  /* compute width of substring in user units, add it */
	  width += _label_width_standard_string (s);
	  free (s);
	}
    }
  
  /* restore original font and font size */
  if (changed_font)
    {
      free (_drawstate->font_name);
      _drawstate->font_name = initial_font_name;
      _drawstate->font_size = initial_font_size;
      _retrieve_font();		/* initial_font_name will be freed later */
      _drawstate->font_type = initial_font_type;
    }
  else
    free (initial_font_name);
  
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

/* _alabel_standard_internal() prints a single-font, single-font-size
   substring of a label, and repositions to the end of the substring after
   printing.  When this is called, the current point is on the intended
   baseline of the string.  And the font should already be selected. */

static void
_alabel_standard_internal (s)
     const unsigned char *s;
{
  double x, y, width;
  double theta, costheta, sintheta;
  
  if (*s == (unsigned char)'\0')
    return;

  /* compute position in device coordinates */
  x = XD((_drawstate->pos).x, (_drawstate->pos).y);
  y = YD((_drawstate->pos).x, (_drawstate->pos).y);
  
  if (_drawstate->native_positioning)
    {
      /* a special case: the font name did not include a pixel matrix, or
         it did but the text rotation angle is zero; so move the easy way,
         i.e., use native repositioning */
      int label_len = strlen ((char *)s);

      XDrawString (_xdata.dpy, _xdata.window, _drawstate->gc, 
		   IROUND(x), IROUND(y), (char *)s, label_len);
      XDrawString (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		   IROUND(x), IROUND(y), (char *)s, label_len);
    }  
  else 
    {
      /* general case: due to nonzero text rotation and/or a non-uniform
	 transformation from user to device coordinates, a pixel matrix
	 appeared explicitly in the font name (and hence the font name was
	 an XLFD font name); must move the cursor and plot each character
	 individually.  */
      double offset = 0.0;
      const unsigned char *stringptr = s;
      
      while (*stringptr)
	{
	  int charno = *stringptr;
	  int char_metric_offset = 
	    charno - _drawstate->x_font_struct->min_char_or_byte2;
	  
	  XDrawString (_xdata.dpy, _xdata.window, _drawstate->gc, 
		       IROUND(x + offset * _drawstate->font_pixmatrix[0] / 1000.0),
		       IROUND(y - offset * _drawstate->font_pixmatrix[1] / 1000.0),
		       (char *)stringptr, 1);
	  XDrawString (_xdata.dpy, _xdata.pixmap, _drawstate->gc, 
		       IROUND(x + offset * _drawstate->font_pixmatrix[0] / 1000.0),
		       IROUND(y - offset * _drawstate->font_pixmatrix[1] / 1000.0),
		       (char *)stringptr, 1);
	  
	  stringptr++;
	  offset += (double)(_drawstate->x_font_struct->per_char ?
			     _drawstate->x_font_struct->per_char[char_metric_offset].attributes :
			     _drawstate->x_font_struct->min_bounds.attributes);
	}
    }
  
  /* width of the substring in user units */
  width = _label_width_standard_string (s);

  /* label rotation angle in radians */
  theta = M_PI * _drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* reposition after printing substring */
  (_drawstate->pos).x += costheta * width;
  (_drawstate->pos).y += sintheta * width;

  return;
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
  double initial_user_font_size = _drawstate->x_font_size;
  char *initial_font_name;
  double initial_font_size = _drawstate->font_size;
  double initial_position_x = (_drawstate->pos).x;
  double initial_position_y = (_drawstate->pos).y;
  double pushed_position_x = (_drawstate->pos).x; /* user can push */
  double pushed_position_y = (_drawstate->pos).y;
  int master_font_index;
  Boolean changed_font = FALSE;
  int initial_font_type = _drawstate->font_type;

  /* select our foreground color */
  _evaluate_x_fgcolor();
  XSetForeground (_xdata.dpy, _drawstate->gc, _drawstate->x_fgcolor);
  
  if (_drawstate->x_font_struct == NULL)
    {
      fprintf (stderr, 
	       "libplot: warning: not drawing a label, since there is no current font\n");
      _handle_x_events();
      return 0.0;
    }

  /* determine index of font, for comparison with font annotations in string */
  switch (_drawstate->font_type)
    {
    case F_STANDARD:
    default:
      master_font_index =
	(_ps_typeface_info[_drawstate->typeface_index].fonts)[_drawstate->font_index];
      break;
    case F_DEVICE_SPECIFIC:
      master_font_index = 1;	/* `1' just means the font we start out with */
      break;
    }

  /* save font name */
  initial_font_name = 
    (char *)_plot_xmalloc (1 + strlen (_drawstate->font_name));
  strcpy (initial_font_name, _drawstate->font_name);

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
      break;

    case 'c': /* centered */
      x_offset = -0.5;
      x_displacement = 0.0;
      /* range [-0.5,0.5] */
      break;

    case 'r': /* right justified */
      x_offset = -1.0;
      x_displacement = -1.0;
      /* range [-1,0] */
      break;
    }

  /* need these to compute offset for vertical justification */
  ascent = (double)(_drawstate->font_raw_ascent);
  descent = (double)(_drawstate->font_raw_descent);

  switch (y_justify_c)		/* placement of label with respect
				   to y coordinate */
    {
    case 'b':			/* current point is at bottom */
      y_offset = descent / 1000.0;
      break;

    case 'x':			/* current point is on baseline */
    default:
      y_offset = 0.0;
      break;

    case 'c':			/* current point is midway between bottom, top */
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
  while (*cptr)			/* end when (unsigned short)0 is seen */
    {
      unsigned short c;

      c = *cptr;
      if (c & CONTROL_CODE)
	{
	  double user_font_size = _drawstate->x_font_size;

	  /* parse control code */
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      (_drawstate->pos).x += (costheta * SUBSCRIPT_DX 
				      - sintheta * SUBSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y += (sintheta * SUBSCRIPT_DX
				      + costheta * SUBSCRIPT_DY) * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      (_drawstate->pos).x += (costheta * SUPERSCRIPT_DX 
				      - sintheta * SUPERSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y += (sintheta * SUPERSCRIPT_DX
				      + costheta * SUPERSCRIPT_DY) * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      break;

	    case C_END_SUBSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      user_font_size = _drawstate->x_font_size;
	      (_drawstate->pos).x -= (costheta * SUBSCRIPT_DX 
				      - sintheta * SUBSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y -= (sintheta * SUBSCRIPT_DX
				      + costheta * SUBSCRIPT_DY) * user_font_size;
	      break;

	    case C_END_SUPERSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      changed_font = TRUE;
	      user_font_size = _drawstate->x_font_size;
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

      else			/* an ordinary char, with font annotation */
	{
	  unsigned char *s, *sptr;
	  int new_master_font_index = (c >> FONT_SHIFT) & ONE_BYTE;

	  /* perform font switching if requested */
	  if (new_master_font_index != master_font_index)
	    {
	      free (_drawstate->font_name);
	      switch (_drawstate->font_type)
		{
		case F_STANDARD:
		default:
		  _drawstate->font_name =
		    (char *)_plot_xmalloc(1 + strlen (_ps_font_info[new_master_font_index].x_name));
		  strcpy (_drawstate->font_name, _ps_font_info[new_master_font_index].x_name);
		  break;
		case F_DEVICE_SPECIFIC:
		  if (new_master_font_index == 0) /* symbol font */
		    {
		      _drawstate->font_name =
			(char *)_plot_xmalloc(1 + strlen (X_SYMBOL_FONT));
		      strcpy (_drawstate->font_name, X_SYMBOL_FONT);
		    }
		  else		/* 1, i.e., restore original font */
		    {
		      _drawstate->font_name =
			(char *)_plot_xmalloc(1 + strlen (initial_font_name));
		      strcpy (_drawstate->font_name, initial_font_name);
		    }
		  break;
		}
	      _retrieve_font();
	      _drawstate->font_type = initial_font_type;
	      master_font_index = new_master_font_index;
	      changed_font = TRUE;
	    }

	  /* extract substring consisting of characters in the same font */
	  sptr = s 
	    = (unsigned char *)_plot_xmalloc ((unsigned int)(4 * _codestring_len (cptr) + 1));
	  while (*cptr 
		 && (*cptr & CONTROL_CODE) == 0 
		 && ((*cptr >> FONT_SHIFT) & ONE_BYTE) == master_font_index)
	    *sptr++ = (*cptr++) & ONE_BYTE;
	  *sptr = (unsigned char)'\0';

	  /* plot the substring, and free it */
	  _alabel_standard_internal (s);
	  free (s);
	}
    }

  /* restore position to what it was before printing label */
  (_drawstate->pos).x = initial_position_x;
  (_drawstate->pos).y = initial_position_y;

  /* restore original font and font size */
  if (changed_font)
    {
      free (_drawstate->font_name);
      _drawstate->font_name = initial_font_name;
      _drawstate->font_size = initial_font_size;
      _retrieve_font();		/* initial_font_name will be freed later */
      _drawstate->font_type = initial_font_type;
    }
  else
    free (initial_font_name);

#if 1				/* 1 will become WIDELABEL */
  /* shift after printing label, if desired (user units) */
  (_drawstate->pos).x += costheta * x_displacement * overall_width;
  (_drawstate->pos).y += sintheta * x_displacement * overall_width;
#endif

  _handle_x_events();

  /* return length of string in user units */
  return overall_width;
}

/* For libplotX, once we have an XFontStruct there's no great difference
   between drawing a label in a non-standard (``device-specific'') font,
   and drawing it in a standard font (one of the 35 fonts listed in
   fontdb.c).  The only difference is that font annotations (shifts between
   fonts within a single typeface) are ignored, since we have no
   information on what the other fonts within the font's typeface are. */

double
_alabel_device (x_justify, y_justify, codestring)
     int x_justify, y_justify;
     const unsigned short *codestring;
{
  return _alabel_standard (x_justify, y_justify, codestring);
}

double
_labelwidth_device (codestring)
     const unsigned short *codestring;
{
  return _labelwidth_standard (codestring);
}
