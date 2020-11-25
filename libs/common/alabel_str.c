/* This file contains the internal function _alabel_stroke(), which plots a
   label using vector fonts.  Each character in a vector font is a sequence
   of pen motions, so this function simply calls fmoverel() and fcontrel()
   to `stroke' each character in the argument string.

   The width of the string in user units is returned. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "her_metrics.h"

/* Shearing factor for oblique fonts, new_x = x + SHEAR * y  */

#define SHEAR (2.0/7.0)

/* Relative size of subscripts/superscripts (i.e. `indexical' size) */

#define SCRIPTSIZE (0.6)	

/* Positioning of subscripts/superscripts */

#define SUBSCRIPT_DX 0.0
#define SUBSCRIPT_DY (-0.25)
#define SUPERSCRIPT_DX 0.0
#define SUPERSCRIPT_DY 0.4

/* Positioning of accents (in Hershey units).  UP_SHIFT is amount by which
   accents are raised when placed over upper-case letters.  RIGHT_SHIFT is
   applied as well, if the upper-case letter is italic. */

#define ACCENT_UP_SHIFT 7.0
#define ACCENT_RIGHT_SHIFT 2.0

/* Relative size of small Japanese Kana */
#define SMALL_KANA_SIZE 0.725

/* Hershey glyph arrays */

#define OCCIDENTAL 0
#define ORIENTAL 1

/* Position of `undefined' glyph in occidental array */

#define UNDE 4023

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
double _label_width_stroke P__((const unsigned short *label));
void _draw_hershey_glyph P__((int num, double charsize, int type, Boolean oblique));
void _draw_hershey_penup_stroke P__((double dx, double dy, double charsize, Boolean oblique));
void _draw_hershey_string P__((const unsigned short *string));
void _draw_hershey_stroke P__((Boolean pendown, double deltax, double deltay));
void _draw_stroke P__((Boolean pendown, double deltax, double deltay));
Boolean _composite_char P__((unsigned char *composite, unsigned char *character, unsigned char *accent));
#undef P__

/* _draw_hershey_stroke() draws a stroke, taking into account the
   transformation from Hershey units to user units, and also the current
   transformation matrix (as set by the user).  _draw_stroke is similar,
   but takes arguments in user units. */

void
_draw_hershey_stroke (pendown, deltax, deltay)
     Boolean pendown;
     double deltax, deltay;
{
  _draw_stroke (pendown,
		HERSHEY_UNITS_TO_USER_UNITS (deltax),
		HERSHEY_UNITS_TO_USER_UNITS (deltay));
}

void
_draw_stroke (pendown, deltax, deltay)
     Boolean pendown;
     double deltax, deltay;
{
  double dx, dy;
  double theta;
  
  theta = M_PI * _drawstate->text_rotation / 180.0;
  
  dx = cos(theta) * deltax - sin(theta) * deltay;
  dy = sin(theta) * deltax + cos(theta) * deltay;

  if (pendown)
    fcontrel (dx, dy);
  else
    fmoverel (dx, dy);
}

double
_labelwidth_stroke (codestring)
     const unsigned short *codestring;
{
  double label_width;
  
  label_width = _label_width_stroke (codestring);
  
  return label_width;
}

double
_alabel_stroke (x_justify, y_justify, codestring)
     int x_justify, y_justify;
     const unsigned short *codestring;
{
  char x_justify_c, y_justify_c;
  double label_width, label_height;
  double x_offset, y_offset;
  double x_displacement;
  double postdx, dx, dy;
  double theta;

  /* dimensions of the string, in user units */
  label_width = _label_width_stroke (codestring);
  label_height = HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_HEIGHT);
  
  x_justify_c = (char)x_justify;
  y_justify_c = (char)y_justify;  

  switch (x_justify_c)
    {
    case 'l': /* left justified */
    default:
      x_offset = 0.0;
      x_displacement = 1.0;
      break;

    case 'c': /* centered */
      x_offset = -0.5;
      x_displacement = 0.0;
      break;

    case 'r': /* right justified */
      x_offset = -1.0;
      x_displacement = -1.0;
      break;
    }

  switch (y_justify_c)
    {
    case 'b':			/* current point is at bottom */
      y_offset = (double)HERSHEY_DESCENT / (double)HERSHEY_HEIGHT;
      break;

    case 'x':			/* current point is on baseline */
    default:
      y_offset = 0.0;
      break;

    case 'c': 			/* current point is midway between bottom, top */
      y_offset = 0.5 * ((double)HERSHEY_DESCENT - (double)HERSHEY_ASCENT) 
		   / (double)HERSHEY_HEIGHT;
      break;

    case 't':			/* current point is at top */
      y_offset = - (double)HERSHEY_ASCENT / (double)HERSHEY_HEIGHT;
      break;
    }

  /* save five relevant drawing attributes, and restore them later */
  {
    char *old_line_mode, *old_cap_mode, *old_join_mode;
    double old_line_width;
    int old_fill_level;
    double oldposx, oldposy;

    old_line_mode = (char *)_plot_xmalloc (strlen (_drawstate->line_mode) + 1);
    old_cap_mode = (char *)_plot_xmalloc (strlen (_drawstate->cap_mode) + 1);
    old_join_mode = (char *)_plot_xmalloc (strlen (_drawstate->join_mode) + 1);
    oldposx = (_drawstate->pos).x;
    oldposy = (_drawstate->pos).y;    

    strcpy (old_line_mode, _drawstate->line_mode);
    strcpy (old_cap_mode, _drawstate->cap_mode);
    strcpy (old_join_mode, _drawstate->join_mode);
    old_line_width = _drawstate->line_width;
    old_fill_level = _drawstate->fill_level;
    
    /* Our choices for rendering: solid lines, rounded capitals and joins,
       a line width equal to slightly more than 1 Hershey unit, and
       transparent filling. */
    linemod ("solid");
    capmod ("butt");		/* options: butt/round/projecting */
    joinmod ("round");		/* options: miter/round/bevel */
    flinewidth (HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_STROKE_WIDTH));
    fill (0);
    
    /* move to take horizontal and vertical justification into account;
       arguments here are in user units */
    _draw_stroke (FALSE, 
		 x_offset * label_width, y_offset * label_height);
    
    /* call stroker on the sequence of strokes obtained from each char */
    _draw_hershey_string (codestring);
    
    /* Restore original values of the drawing attributes, free storage.
       endpoly() will be invoked in here automatically, flushing the
       created polyline object comprising the stroked text. */
    linemod (old_line_mode);
    capmod (old_cap_mode);
    joinmod (old_join_mode);
    flinewidth (old_line_width);
    fill (old_fill_level);
    
    free (old_line_mode);
    free (old_cap_mode);
    free (old_join_mode);

    /* return to original position */
    fmove (oldposx, oldposy);
  }

#if 1				/* 1 will become WIDELABEL */
  /* amount by which to shift after printing label, if desired (user units) */
  postdx = x_displacement * label_width;
  theta = M_PI * _drawstate->text_rotation / 180.0;
  dx = cos (theta) * postdx
    - sin (theta) * 0;
  dy = sin (theta) * postdx
    + cos (theta) * 0;

  fmoverel (dx, dy);
#endif

  return label_width;		/* user units */
}

/* In addition to scaling the character sizes and the `width', we perform
   the following (dx, dy):
   
   enter subscript	(dx, dy) = (-1/9, -1/2) * width
   exit subscript	(dx, dy) = (+1/6, +1/2) * width
   
   enter superscript	(dx, dy) = (-1/9, +1/2) * width
   exit superscript	(dx, dy) = (+1/6, -1/2) * width
   
   For clarity here, `width' refers to the width _before_ it is
   multiplied by a factor 2/3.
   
   [N.B. In Bob Beach's original UGS character stroke generator,
   the +1/6's here were +2/9 instead.  Better?] */

/* _label_width_stroke() computes the width (total delta x) of a
   controlified character string to be rendered in a vector font, in user
   units */
double
_label_width_stroke (label) 
     const unsigned short *label;
{ 
  const unsigned short *ptr = label;
  unsigned short c;
  double charsize = 1.0;	/* relative char size, 1.0 means full size */
  double saved_charsize = 1.0;
  double width = 0.0;		/* label width */
  double saved_width = 0.0;
  
  /* loop through unsigned shorts in label */
  while ((c = (*ptr)) != (unsigned short)'\0') 
    {
      int glyphnum;		/* glyph in Hershey array */
      unsigned char *glyph;
      
      if (c & RAW_HERSHEY_GLYPH) 
	/* glyph was spec'd via an escape, not as a char in a font */
	{
	  glyphnum = c & GLYPH_SPEC;
	  glyph = (unsigned char *)(_occidental_vector_glyphs[glyphnum]);
	  
	  if (*glyph != '\0')	/* nonempty glyph */
	    /* 1st two chars are bounds */
	    width += charsize * ((int)glyph[1] - (int)glyph[0]);
	}
      else if (c & RAW_ORIENTAL_HERSHEY_GLYPH) 
	/* glyph was spec'd via an escape, not as a char in a font */
	{
	  glyphnum = c & GLYPH_SPEC;
	  glyph = (unsigned char *)_oriental_vector_glyphs[glyphnum];
	  
	  if (*glyph != '\0')	/* nonempty glyph */
	    /* 1st two chars are bounds */
	    width += charsize * ((int)glyph[1] - (int)glyph[0]);
	}
      else if (c & CONTROL_CODE)	/* parse control code */
	{
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	    case C_BEGIN_SUPERSCRIPT :
	      charsize *= SCRIPTSIZE;
	      break;
	      
	    case C_END_SUBSCRIPT:
	    case C_END_SUPERSCRIPT:
	      charsize /= SCRIPTSIZE;
	      break;
	      
	    case C_PUSH_LOCATION:
	      saved_width = width;
	      saved_charsize = charsize;
	      break;
	      
	    case C_POP_LOCATION:
	      width = saved_width;
	      charsize = saved_charsize;
	      break;
	      
	    case C_RIGHT_ONE_EM:
	      width += charsize * HERSHEY_EM;
	      break;
	      
	    case C_RIGHT_HALF_EM:
	      width += charsize * HERSHEY_EM / 2.0;
	      break;
	      
	    case C_RIGHT_QUARTER_EM:
	      width += charsize * HERSHEY_EM / 4.0;
	      break;
	      
	    case C_RIGHT_SIXTH_EM:
	      width += charsize * HERSHEY_EM / 6.0;
	      break;
	      
	    case C_RIGHT_EIGHTH_EM:
	      width += charsize * HERSHEY_EM / 8.0;
	      break;
	      
	    case C_LEFT_ONE_EM:
	      width -= charsize * HERSHEY_EM;
	      break;
	      
	    case C_LEFT_HALF_EM:
	      width -= charsize * HERSHEY_EM / 2.0;
	      break;
	      
	    case C_LEFT_QUARTER_EM:
	      width -= charsize * HERSHEY_EM / 4.0;
	      break;
	      
	    case C_LEFT_SIXTH_EM:
	      width -= charsize * HERSHEY_EM / 6.0;
	      break;
	      
	    case C_LEFT_EIGHTH_EM:
	      width -= charsize * HERSHEY_EM / 8.0;
	      break;
	      
	      /* unrecognized control code */
	    default:
	      break;
	    }
	}
      else			/* yow, an actual character */
	{
	  int raw_fontnum;
	  
	  /* compute index of font, in table in fontdb.c */
	  raw_fontnum = (c >> FONT_SHIFT) & ONE_BYTE;
	  
	  c &= ~FONT_SPEC;	/* extract character proper */
	  glyphnum = (_vector_font_info[raw_fontnum].chars)[c];

	  /* could be a pseudo glyph number, e.g. an indication that
	     character is composite */
	  if (glyphnum == ACC0 || glyphnum == ACC1 || glyphnum == ACC2)
	    {
	      unsigned char composite, character, accent;

	      /* if so, use 1st element of composite character */
	      composite = (unsigned char)c;
	      if (_composite_char (&composite, &character, &accent))
		glyphnum = (_vector_font_info[raw_fontnum].chars)[character];
	      else
		glyphnum = UNDE; /* hope this won't happen */
	    }

	  /* could also be a glyph number displaced by KS, to indicate
	     that this is a small kana */
	  if (glyphnum & KS)
	    glyphnum -= KS;

	  glyph = (unsigned char *)(_occidental_vector_glyphs[glyphnum]);
	  if (*glyph != '\0')	/* nonempty glyph */
	    /* 1st two chars are bounds */
	    width += charsize * ((int)glyph[1] - (int)glyph[0]);
	}
      
      ptr++;			/* bump pointer in string */
    }

  return HERSHEY_UNITS_TO_USER_UNITS (width);
}  

/* _draw_hershey_penup_stroke() draws a penup stroke, along a vector
   specified in Hershey units.  Size scaling and obliquing (true/false) are
   specified.  This is used for repositioning during rendering of
   composite (accented) characters. */
void
_draw_hershey_penup_stroke(dx, dy, charsize, oblique)
     double dx, dy;
     double charsize;
     Boolean oblique;
{
  double shear;

  shear = oblique ? (SHEAR) : 0.0;
  _draw_hershey_stroke (FALSE,	/* pen up */
			charsize * (dx + shear * dy), 
			charsize * dy);
}

/* _draw_hershey_glyph() draws a raw Hershey glyph, specified by index in
   the occidental or oriental glyph arrays.  Size scaling and obliquing
   (true/false) are specified. */
void
_draw_hershey_glyph (glyphnum, charsize, type, oblique)
     int glyphnum;
     double charsize;
     int type;
     Boolean oblique;
{
  double xcurr, ycurr;
  double xfinal, yfinal;
  Boolean pendown = FALSE;
  unsigned char *glyph;
  double dx, dy;
  double shear;
  
  shear = oblique ? (SHEAR) : 0.0;
  switch (type)
    {
    case OCCIDENTAL:
    default:
      glyph = (unsigned char *)(_occidental_vector_glyphs[glyphnum]);
      break;
    case ORIENTAL:
      glyph = (unsigned char *)(_oriental_vector_glyphs[glyphnum]); 
      break;
    }

  if (*glyph != '\0')	/* nonempty glyph */
    {
      xcurr = charsize * (double)glyph[0];
      xfinal = charsize * (double)glyph[1];
      ycurr = yfinal = 0.0;
      glyph += 2;
      while (*glyph)
	{
	  int xnewint;
	  
	  xnewint = (int)glyph[0];
	  
	  if (xnewint == (int)' ')
	    pendown = FALSE;
	  else
	    {
	      double xnew, ynew;

	      xnew = (double)charsize * xnewint;
	      ynew = (double)charsize 
		* ((int)'R' 
		   - ((int)glyph[1] + (double)HERSHEY_BASELINE));
	      dx = xnew - xcurr;
	      dy = ynew - ycurr;
	      _draw_hershey_stroke (pendown, dx + shear * dy, dy);
	      xcurr = xnew, ycurr = ynew;
	      pendown = TRUE;
	    }
	  
	  glyph +=2;	/* on to next pair */
	}
      
      /* final penup stroke, to end where we should */
      dx = xfinal - xcurr;
      dy = yfinal - ycurr;
      _draw_hershey_stroke (FALSE, dx + shear * dy, dy);
    }
}

/* _draw_hershey_string() will stroke a string beginning at the present
   location, which will be taken to be on the string's baseline */
void
_draw_hershey_string (string)
     const unsigned short *string;
{
  unsigned short c;
  const unsigned short *ptr = string;
  double charsize = 1.0;
  double saved_charsize = 1.0;
  double saved_position_x = (_drawstate->pos).x;
  double saved_position_y = (_drawstate->pos).y;
  
  while ((c = (*ptr)) != '\0') 
    {
      if (c & RAW_HERSHEY_GLYPH)
	_draw_hershey_glyph (c & GLYPH_SPEC, charsize, OCCIDENTAL, FALSE);

      else if (c & RAW_ORIENTAL_HERSHEY_GLYPH)
	_draw_hershey_glyph (c & GLYPH_SPEC, charsize, ORIENTAL, FALSE);

      else	/* not a raw Hershey glyph; a control code or a genuine char */
	{
	  if (c & CONTROL_CODE)	
	    switch (c & ~CONTROL_CODE) /* parse control codes */
	      {
	      case C_BEGIN_SUPERSCRIPT :
		_draw_hershey_stroke (FALSE, 
				      SUPERSCRIPT_DX * charsize * HERSHEY_EM,
				      SUPERSCRIPT_DY * charsize * HERSHEY_EM);
		charsize *= SCRIPTSIZE;
		break;
		
	      case C_END_SUPERSCRIPT:
		charsize /= SCRIPTSIZE;
		_draw_hershey_stroke (FALSE, 
				      - SUPERSCRIPT_DX * charsize * HERSHEY_EM,
				      - SUPERSCRIPT_DY * charsize * HERSHEY_EM);
		break;
		
	      case C_BEGIN_SUBSCRIPT:
		_draw_hershey_stroke (FALSE, 
				      SUBSCRIPT_DX * charsize * HERSHEY_EM,
				      SUBSCRIPT_DY * charsize * HERSHEY_EM);
		charsize *= SCRIPTSIZE;
		break;
		
	      case C_END_SUBSCRIPT:
		charsize /= SCRIPTSIZE;
		_draw_hershey_stroke (FALSE, 
				      - SUBSCRIPT_DX * charsize * HERSHEY_EM,
				      - SUBSCRIPT_DY * charsize * HERSHEY_EM);
		break;
		
	      case C_PUSH_LOCATION:
		saved_charsize = charsize;
		saved_position_x = (_drawstate->pos).x;
		saved_position_y = (_drawstate->pos).y;
		break;
		
	      case C_POP_LOCATION:
		charsize = saved_charsize;
		fmove (saved_position_x, saved_position_y);
		break;
		
	      case C_RIGHT_ONE_EM:
		_draw_hershey_stroke (FALSE, charsize * HERSHEY_EM, 0.0);
		break;
		
	      case C_RIGHT_HALF_EM:
		_draw_hershey_stroke (FALSE, charsize * HERSHEY_EM / 2.0, 0.0);
		break;
		
	      case C_RIGHT_QUARTER_EM:
		_draw_hershey_stroke (FALSE, charsize * HERSHEY_EM / 4.0, 0.0);
		break;
		
	      case C_RIGHT_SIXTH_EM:
		_draw_hershey_stroke (FALSE, charsize * HERSHEY_EM / 6.0, 0.0);
		break;
		
	      case C_RIGHT_EIGHTH_EM:
		_draw_hershey_stroke (FALSE, charsize * HERSHEY_EM / 8.0, 0.0);
		break;
		
	      case C_LEFT_ONE_EM:
		_draw_hershey_stroke (FALSE, - charsize * HERSHEY_EM, 0.0);
		break;
		
	      case C_LEFT_HALF_EM:
		_draw_hershey_stroke (FALSE, - charsize * HERSHEY_EM / 2.0, 0.0);
		break;
		
	      case C_LEFT_QUARTER_EM:
		_draw_hershey_stroke (FALSE, - charsize * HERSHEY_EM / 4.0, 0.0);
		break;
		
	      case C_LEFT_SIXTH_EM:
		_draw_hershey_stroke (FALSE, - charsize * HERSHEY_EM / 6.0, 0.0);
		break;
		
	      case C_LEFT_EIGHTH_EM:
		_draw_hershey_stroke (FALSE, - charsize * HERSHEY_EM / 8.0, 0.0);
		break;
		
		/* unrecognized control code */
	      default:
		break;
	      }
      
	  else			/* yow, an actual character */
	    {
	      int raw_fontnum;
	      int glyphnum;		/* glyph in Hershey array */
	      int char_glyphnum, accent_glyphnum; /* for composite chars */
	      int char_width, accent_width; /* for composite chars */
	      unsigned char *char_glyph, *accent_glyph;
	      unsigned char composite, character, accent;
	      Boolean oblique, small_kana = FALSE;
	      
	      /* compute index of font, in table in fontdb.c */
	      raw_fontnum = (c >> FONT_SHIFT) & ONE_BYTE;
	      /* shear font?  (for HersheySans-Oblique, etc.) */
	      oblique = _vector_font_info[raw_fontnum].obliquing;
	      
	      c &= ~FONT_SPEC;	/* extract character proper */
	      glyphnum = (_vector_font_info[raw_fontnum].chars)[c];

	      if (glyphnum & KS)
		{
		  glyphnum -= KS;
		  small_kana = TRUE;
		}

	      switch (glyphnum)
		{
		  /* special case: this is a composite (accented)
                     character; search table in fontdb.c for it */
		case ACC0:
		case ACC1:
		case ACC2:
		  composite = (unsigned char)c;
		  if (_composite_char (&composite, &character, &accent))
		    {
		      char_glyphnum = 
			(_vector_font_info[raw_fontnum].chars)[character];
		      accent_glyphnum = 
			(_vector_font_info[raw_fontnum].chars)[accent];
		    }
		  else
		    {		/* hope this won't happen */
		      char_glyphnum = UNDE; 
		      accent_glyphnum = 0;
		    }
		  char_glyph = 
		    (unsigned char *)_occidental_vector_glyphs[char_glyphnum];
		  accent_glyph = 
		    (unsigned char *)_occidental_vector_glyphs[accent_glyphnum];
	  
		  if (*char_glyph != '\0')	/* nonempty glyph */
		    /* 1st two chars are bounds, in Hershey units */
		    char_width = (int)char_glyph[1] - (int)char_glyph[0];
		  else
		    char_width = 0;

		  if (*accent_glyph != '\0')	/* nonempty glyph */
		    /* 1st two chars are bounds, in Hershey units */
		    accent_width = (int)accent_glyph[1] - (int)accent_glyph[0];
		  else
		    accent_width = 0;

		  /* draw the character */
		  _draw_hershey_glyph (char_glyphnum, charsize, 
				       OCCIDENTAL, oblique);
		  /* back up */
		  _draw_hershey_penup_stroke (-0.5 * (double)char_width
					      -0.5 * (double)accent_width,
					      0.0, charsize, oblique);

		  /* repositioning for uppercase and uppercase italic */
		  if (glyphnum == ACC1)
		    _draw_hershey_penup_stroke (0.0, 
						(double)(ACCENT_UP_SHIFT),
						charsize, oblique);
		  else if (glyphnum == ACC2)
		    _draw_hershey_penup_stroke ((double)(ACCENT_RIGHT_SHIFT),
						(double)(ACCENT_UP_SHIFT),
						charsize, oblique);

		  /* draw the accent */
		  _draw_hershey_glyph (accent_glyphnum, charsize, 
				       OCCIDENTAL, oblique);

		  /* undo special repositioning if any */
		  if (glyphnum == ACC1)
		    _draw_hershey_penup_stroke (0.0, 
						-(double)(ACCENT_UP_SHIFT),
						charsize, oblique);
		  else if (glyphnum == ACC2)
		    _draw_hershey_penup_stroke (-(double)(ACCENT_RIGHT_SHIFT),
						-(double)(ACCENT_UP_SHIFT),
						charsize, oblique);

		  /* move forward, to end composite char where we should */
		  _draw_hershey_penup_stroke (0.5 * (double)char_width
					      -0.5 * (double)accent_width,
					      0.0, charsize, oblique);
		  break;

		  /* not a composite character; just an ordinary glyph,
		     though it could be a small Kana */
		default:
		  if (small_kana)
		    {
		      int kana_width;
		      unsigned char *kana_glyph;
		      double shift = 0.5 * (1.0 - (SMALL_KANA_SIZE));

		      kana_glyph = 
			(unsigned char *)_occidental_vector_glyphs[glyphnum];
		      kana_width = (int)kana_glyph[1] - (int)kana_glyph[0];

		      /* draw small Kana, preceded and followed by a penup
                         stroke in order to traverse the full width of an
                         ordinary Kana */
		      _draw_hershey_penup_stroke (shift * (double)kana_width,
						  0.0, charsize, oblique);
		      _draw_hershey_glyph (glyphnum, 
					   (SMALL_KANA_SIZE) * charsize,
					   OCCIDENTAL, oblique);
		      _draw_hershey_penup_stroke (shift * (double)kana_width,
						  0.0, charsize, oblique);
		    }
		  else
		    _draw_hershey_glyph (glyphnum, charsize, 
					 OCCIDENTAL, oblique);
		  break;
		}
	    }
	}
      
      ptr++;			/* bump pointer in controlified string */
    }
  
  return;
}

/* retrieve the two elements of a composite character from the table in
   fontdb.c */
Boolean
_composite_char (composite, character, accent)
     unsigned char *composite;
     unsigned char *character, *accent;
{
  struct accented_char_info_struct *compchar = _vector_accented_char_info;
  Boolean found = FALSE;
  unsigned char given = *composite;
  
  while (compchar->composite)
    {
      if (compchar->composite == given)
	{
	  found = TRUE;
	  /* return char and accent via pointers */
	  *character = compchar->character;
	  *accent = compchar->accent;
	}
      compchar++;
    }

  return found;
}
