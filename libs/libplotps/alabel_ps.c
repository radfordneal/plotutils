/* This file contains the device-specific versions of alabel(), called
   _alabel_standard() and _alabel_device().  This version is for libplotps.

   The width of the string in user units is returned. */

/* A note on font scaling.  idraw scales the whole drawing down by a factor
   of SCALING.  To agree with this, we use a global PS transformation
   matrix equal to [SCALING 0 0 SCALING 0 0] (see closepl.c).

   As a consequence of this scaling, to get X fonts of the right size on an
   idraw display, one would think that we must tell idraw to use an X
   fontsize equal to the fontsize in device units (previously transformed
   from user units, of course), times a factor equal to SCALING.  However,
   versions 2.5+ of idraw don't require such a factor.  See comment in the
   code below. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

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
static double _label_width_standard_codestring P__((const unsigned short *codestring));
static double _label_width_standard_string P__((const unsigned char *s));
static void _alabel_standard_internal P__((const unsigned char *s, int master_font_index));
#undef P__

/* _label_width_standard_string() computes the width (total delta x) of a
   character string to be rendered in one of the 35 standard fonts, in user
   units.  The string is just a string: a sequence of bytes (no font
   annotations, and no control codes). */

static double
_label_width_standard_string (s)
     const unsigned char *s;
{
  double user_font_size = _drawstate->font_size;
  int index;
  int width = 0;
  unsigned char current_char;
  int master_font_index;	/* index into master table */

  /* compute font index in master PS font table */
  master_font_index =
    (_ps_typeface_info[_drawstate->typeface_index].fonts)[_drawstate->font_index];

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
	  double user_font_size = _drawstate->font_size;

	  /* parse control code */
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      width += SUBSCRIPT_DX * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      width += SUPERSCRIPT_DX * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      break;

	    case C_END_SUBSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      width -= SUBSCRIPT_DX * _drawstate->font_size;
	      break;
	      
	    case C_END_SUPERSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      width -= SUPERSCRIPT_DX * _drawstate->font_size;
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
      else			/* an ordinary character */
	{
	  unsigned char current_char;
	  int master_font_index;	/* index into master table */
	  double user_font_size = _drawstate->font_size;

	  /* extract font index from annotation */
	  master_font_index = (c >> FONT_SHIFT) & ONE_BYTE;

	  /* extract character, add its width */
	  current_char = c & ONE_BYTE; 
	  width 
	    += user_font_size 
	      * ((_ps_font_info[master_font_index]).width)[current_char] 
		/ 1000.0;
	}

      cptr++;			/* on to next element of codestring */
    }

  /* restore original font size */
  _drawstate->font_size = saved_font_size;

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
   baseline of the string.  */

static void
_alabel_standard_internal (s, master_font_index)
     const unsigned char *s;
     int master_font_index;
{
  int i;
  double width;
  unsigned char *ptr;
  double theta, costheta, sintheta;
  double norm;
  double dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3;
  double font_ascent, font_descent, up, down;
  double user_font_size = _drawstate->font_size;
  double device_font_size;
  double user_text_transformation_matrix[6];
  double text_transformation_matrix[6];
  Boolean crockflag;
  
  if (*s == (unsigned char)'\0')
    return;

  /* flag current font as used */
  _ps_font_info[master_font_index].used = TRUE;

  /* label rotation angle in radians */
  theta = M_PI * _drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* font ascent and descent (taken from the font's bounding box) */
  font_ascent = (double)((_ps_font_info[master_font_index]).font_ascent);
  font_descent = (double)((_ps_font_info[master_font_index]).font_descent);
  up = user_font_size * font_ascent / 1000.0;
  down = user_font_size * font_descent / 1000.0;

  /* Current point is on the baseline, but the rendering logic of the PS
     code in the idraw prologue requires us to perform a vertical shift at
     this point.  (We'll undo the vertical shift immediately.) */
  (_drawstate->pos).x -= (user_font_size - down) * sintheta;
  (_drawstate->pos).y += (user_font_size - down) * costheta;

  /* the idraw PS prologue (see header.c) performs a gratuitous vertical
     shift by 1 device unit unless the current font is Courier or
     Courier-Bold */
  if ((strcmp (_ps_font_info[master_font_index].ps_name, "Courier") != 0)
      && (strcmp (_ps_font_info[master_font_index].ps_name, "Courier-Bold") != 0))
    crockflag = TRUE;
  else
    crockflag = FALSE;

  /* this transformation matrix rotates, and translates; it maps (0,0) to
     the origin of the string, in user coordinates */
  user_text_transformation_matrix[0] = costheta;
  user_text_transformation_matrix[1] = sintheta;
  user_text_transformation_matrix[2] = - sintheta;
  user_text_transformation_matrix[3] = costheta;
  user_text_transformation_matrix[4] = (_drawstate->pos).x;
  user_text_transformation_matrix[5] = (_drawstate->pos).y;

  /* undo the abovementioned upward vertical shift */
  (_drawstate->pos).x += (user_font_size - down) * sintheta;
  (_drawstate->pos).y -= (user_font_size - down) * costheta;

  /* Construct a temporary matrix that rotates, translates, and then maps
     to device coordinates.  This matrix transforms from a frame in which
     nominal character sizes are roughly 1 unit in the horizontal and
     vertical directions, to device coordinates. */

  _matrix_product (user_text_transformation_matrix, _drawstate->transform.m,
		   text_transformation_matrix);

  /* Compensate for the gratuitous shift in vertical position, see above.
     This will give us correct vertical positioning in both PS and idraw
     for all fonts other than Courier and Courier-Bold.  Also, it will give
     us correct vertical positioning in idraw for Courier and Courier-Bold.
     Positioning in PS for those two fonts, however, will not be good. */

  text_transformation_matrix[5] -= (crockflag ? 1.0 : 0.0);

  /* We need to extract a quantity we can call a font size in device
     coordinates, for the benefit of idraw (which needs to use an X font).
     This quantity is used by the prologue function SetF also.

     We define this to be user_font_size (the nominal font size in user
     coordinates), times the norm of the linear tranformation contained in
     the temporary matrix we just constructed (the magnitude of its larger
     singular value).  Recall that for any square matrix M, the singular
     values are the square roots of the eigenvalues of the symmetric matrix
     M^t M. */

  norm = _matrix_norm (text_transformation_matrix);

  if (norm == 0.0)		/* avoid division by zero */
    return;

  device_font_size = norm * user_font_size;

  /* Now scale the text transformation matrix so that the linear
     transformation contained in it has unit norm (if there is no shearing,
     it will just be a rotation; if there is no rotation either, it will
     be the identity matrix */

  for (i = 0; i < 4; i++)
    text_transformation_matrix[i] /= norm;

  /* prologue instruction, plus idraw directive: start of Text */
  strcpy (_outbuf.current, "Begin %I Text\n");
  _update_buffer (&_outbuf);

  /* idraw directive, plus prologue instruction: set foreground color */
  _evaluate_ps_fgcolor();	/* _evaluated lazily, i.e. when needed */
  sprintf (_outbuf.current, "%%I cfg %s\n%f %f %f SetCFg\n",
	   _idraw_stdcolornames[_drawstate->idraw_fgcolor],
	   _drawstate->ps_fgcolor_red,
	   _drawstate->ps_fgcolor_green,
	   _drawstate->ps_fgcolor_blue);
  _update_buffer (&_outbuf);

  /* idraw directive: X Windows font name, which incorporates the X font
     size.  Notice that the font size is _not_ multiplied by a factor equal
     to SCALING; current releases of idraw [2.5+] do not need such a
     factor, even though naively one would think it should be present. */

  /* N.B. this directive sets the _pixel_ size of the X font to be our
     current point size.  That would really be appropriate only if the
     screen resolution is 72 dpi.  But idraw seems to prefer setting the
     pixel size to setting the point size. */

  sprintf (_outbuf.current,
	  "%%I f -*-%s-*-%d-*-*-*-*-*-*-*\n", 
	   (_ps_font_info[master_font_index]).x_name, 
	   IROUND(device_font_size));
  _update_buffer (&_outbuf);

  /* prolog instruction: PS font name and size */
  sprintf (_outbuf.current, "/%s %f SetF\n", 
	   _ps_font_info[master_font_index].ps_name,
	   device_font_size);
  _update_buffer (&_outbuf);

  /* idraw directive and prologue instruction: text transformation matrix */
  strcpy (_outbuf.current, "%I t\n[ ");
  _update_buffer (&_outbuf);

  for (i = 0; i < 6; i++)
    {
      sprintf (_outbuf.current, "%f ", text_transformation_matrix[i]);
      _update_buffer (&_outbuf);      
    }
  
  /* width of the substring in user units (used below in constructing a
     bounding box, and in performing repositioning at end) */
  width = _label_width_standard_string (s);

  /* to compute an EPS-style bounding box, first compute offsets to the
     four vertices of the smallest rectangle containing the string */
  dx0 = - sintheta * (-down);
  dy0 =   costheta * (-down);
  
  dx1 = - sintheta * up;
  dy1 =   costheta * up;
  
  dx2 = costheta * width - sintheta * (-down);
  dy2 = sintheta * width + costheta * (-down);
  
  dx3 = costheta * width - sintheta * up;
  dy3 = sintheta * width + costheta * up;

  /* record that we're using all four vertices (args of _set_range() are in
     device units, not user units) */
  _set_range (XD ((_drawstate->pos).x + dx0, (_drawstate->pos).y + dy0),
	      YD ((_drawstate->pos).x + dx0, (_drawstate->pos).y + dy0));
  _set_range (XD ((_drawstate->pos).x + dx1, (_drawstate->pos).y + dy1), 
	      YD ((_drawstate->pos).x + dx1, (_drawstate->pos).y + dy1));
  _set_range (XD ((_drawstate->pos).x + dx2, (_drawstate->pos).y + dy2), 
	      YD ((_drawstate->pos).x + dx2, (_drawstate->pos).y + dy2));
  _set_range (XD ((_drawstate->pos).x + dx3, (_drawstate->pos).y + dy3), 
	      YD ((_drawstate->pos).x + dx3, (_drawstate->pos).y + dy3));

  /* Finish outputting transformation matrix; begin outputting string. */
  /* Escape all backslashes etc. in the text string, before output. */
  strcpy (_outbuf.current, " ] concat\n\
%I\n\
[\n\
(");
  _update_buffer (&_outbuf);

  ptr = (unsigned char *)_outbuf.current;
  while (*s)
    {
      switch (*s)
	{
	case '(':		/* for PS, escape ()/ */
	case ')':
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
  _update_buffer (&_outbuf);

  /* prologue instruction: end of text */
  strcpy (_outbuf.current, ")\n\
] Text\n\
End\n\
\n");
  _update_buffer (&_outbuf);

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
  double initial_font_size = _drawstate->font_size;
  double initial_user_font_size = _drawstate->font_size;
  double initial_position_x = (_drawstate->pos).x;
  double initial_position_y = (_drawstate->pos).y;
  double pushed_position_x = (_drawstate->pos).x; /* user can push */
  double pushed_position_y = (_drawstate->pos).y;
  int master_font_index;	/* index into master table */

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

  /* compute label width, in user units */
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
	  double user_font_size = _drawstate->font_size;

	  /* parse control code */
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      (_drawstate->pos).x += (costheta * SUBSCRIPT_DX 
				      - sintheta * SUBSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y += (sintheta * SUBSCRIPT_DX
				      + costheta * SUBSCRIPT_DY) * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      (_drawstate->pos).x += (costheta * SUPERSCRIPT_DX 
				      - sintheta * SUPERSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y += (sintheta * SUPERSCRIPT_DX
				      + costheta * SUPERSCRIPT_DY) * user_font_size;
	      _drawstate->font_size *= SCRIPTSIZE;
	      break;

	    case C_END_SUBSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      user_font_size = _drawstate->font_size;
	      (_drawstate->pos).x -= (costheta * SUBSCRIPT_DX 
				      - sintheta * SUBSCRIPT_DY) * user_font_size;
	      (_drawstate->pos).y -= (sintheta * SUBSCRIPT_DX
				      + costheta * SUBSCRIPT_DY) * user_font_size;
	      break;

	    case C_END_SUPERSCRIPT:
	      _drawstate->font_size /= SCRIPTSIZE;
	      user_font_size = _drawstate->font_size;
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

	  /* extract the substring consisting of characters in the same
	     font */
	  master_font_index = (c >> FONT_SHIFT) & ONE_BYTE;
	  sptr = s 
	    = (unsigned char *)_plot_xmalloc ((unsigned int)(4 * _codestring_len (cptr) + 1));
	  while (*cptr 
		 && (*cptr & CONTROL_CODE) == 0 
		 && ((*cptr >> FONT_SHIFT) & ONE_BYTE) == master_font_index)
	    *sptr++ = (*cptr++) & ONE_BYTE;
	  *sptr = (unsigned char)'\0';

	  /* plot the substring, and free it */
	  _alabel_standard_internal (s, master_font_index);
	  free (s);
	}
    }

  /* restore original font size */
  _drawstate->font_size = initial_font_size;

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

/* we don't have any device-specific font table yet */
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
