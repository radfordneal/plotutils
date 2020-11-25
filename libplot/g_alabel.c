/* This file contains the alabel method, which is a GNU extension to
   libplot.  It draws a label, i.e. a text string, at the current location.
   Horizontal and vertical justification must be specified.

   ALABEL takes three arguments X_JUSTIFY, Y_JUSTIFY, and S, and places the
   label S according to the x and y axis adjustments specified in X_JUSTIFY
   and Y_JUSTIFY.  X_JUSTIFY is equal to 'l', 'c', or 'r', signifying
   left-justified, centered, or right-justified, relative to the current
   position.  Y_JUSTIFY is equal to 'b', 'x', 'c', or 't', signifying that
   the bottom, baseline, center, or top of the label should pass through
   the current position. */

/* This file also contains the labelwidth method, which is a GNU extension
   to libplot.  It returns the width in user units of a label, i.e., a text
   string. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_control.h"

#define SCRIPTSIZE 0.6		/* rel. size of subscripts/superscripts */

#define SUBSCRIPT_DX 0.0
#define SUBSCRIPT_DY (-0.2)
#define SUPERSCRIPT_DX 0.0
#define SUPERSCRIPT_DY 0.375

/* font we use for symbol escapes if the current font is a user-specified
   one [for X Windows] that doesn't belong to any of our builtin typefaces */
#define SYMBOL_FONT "Symbol"

/* Kludges to handle the zero-width marker symbols in our ArcMath and
   StickMath fonts; also zero-width overbar.
   8 AND 17 ARE HARDCODED IN THE TABLE IN g_fontd2.c !! */
#define ARCMATH 8
#define STICKMATH 17
#define IS_MATH_FONT(fontnum) ((fontnum) == ARCMATH || (fontnum) == STICKMATH)
#define IS_CENTERED_SYMBOL(c) (((c) >= 'A' && (c) <= 'O') || (c) == 'e')

/* forward references */
static unsigned char *_esc_esc_string __P((const unsigned char *s));
static double _g_render_string_non_hershey __P((const char *s, bool do_render, int x_justify, int y_justify));
static double _g_render_simple_string_non_hershey __P((const unsigned char *s, bool do_render, int h_just));
static bool _simple_string __P((const unsigned short *codestring));

/* The generic versions of the flabelwidth() and falabel() methods.  After
   checking for control characters in the input string (not allowed), we
   invoke either a Hershey-specific or a non-Hershey-specific method. */

int
#ifdef _HAVE_PROTOS
_g_alabel (int x_justify, int y_justify, const char *s)
#else
_g_alabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     const char *s;
#endif
{
  char *t;

  if (!_plotter->open)
    {
      _plotter->error ("alabel: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->PointsInLine > 0)
    _plotter->endpath(); /* flush polyline if any */

  if (s == NULL)
    return 0;			/* avoid core dumps */

  /* copy because we may alter the string */
  t = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  /* allow only character set in ISO encoding */
  {
    bool was_clean;
    
    was_clean = _clean_iso_string ((unsigned char *)t);
    if (!was_clean)
      _plotter->warning ("ignoring control character (e.g. CR or LF) in label");
  }
  
  if (_plotter->drawstate->font_type == F_HERSHEY)
    /* call Hershey-specific routine, since controlification acts
       differently (there are more control codes for Hershey strings) */
    _g_falabel_hershey (x_justify, y_justify, (unsigned char *)t);
  else
    /* invoke routine below */
    _g_render_string_non_hershey (t, true, x_justify, y_justify);
  free (t);

  return 0;
}

double
#ifdef _HAVE_PROTOS
_g_flabelwidth (const char *s)
#else
_g_flabelwidth (s)
     const char *s;
#endif
{
  double width = 0.0;
  char *t;

  if (!_plotter->open)
    {
      _plotter->error ("flabelwidth: invalid operation");
      return -1;
    }

  if (s == NULL)
    return 0.0;			/* avoid core dumps */

  /* copy because we may alter the string */
  t = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  /* allow only character set in ISO encoding */
  {
    bool was_clean;
    
    was_clean = _clean_iso_string ((unsigned char *)t);
    if (!was_clean)
      _plotter->warning ("ignoring control character (e.g. CR or LF) in label");
  }
  
  if (_plotter->drawstate->font_type == F_HERSHEY)
    /* call Hershey-specific routine, since controlification acts
       differently (there are more control codes for Hershey strings) */
    width = _g_flabelwidth_hershey ((unsigned char *)t);
  else
    /* invoke routine below; final two args are irrelevant */
    width = _g_render_string_non_hershey (t, false, 'c', 'c');
  free (t);

  return width;
}

/* The non-Hershey version of the falabel() and flabelwidth() methods
   (merged).  They are distinguished by the do_render flag being
   true/false; the return values (the width of the string) are the same.
   The final two arguments, specifying justification, are relevant only if
   the do_render flag is `true'.  If do_render is true, the string is
   rendered in accordance with the justification instructions, and the
   graphics cursor position is updated accordingly. */

/* We `controlify' the string, translating escape sequences to annotations.
   Note: for fonts of `OTHER' type [user-specified X Windows fonts], shifts
   between fonts within a single typeface are ignored, since we have no
   information on what the other fonts within the font's typeface are.  The
   annotations simply indicate whether or not a symbol font should be
   switched to, for the purpose of symbol escapes.  For fonts of `OTHER'
   type, font #1 means the user-specified font and font #0 means the symbol
   font. */

/* As noted, this version is invoked only if the current font is
   non-Hershey.  But due to failure to retrieve an X font, it is possible
   that the font could switch to a Hershey font during rendering. */

static double
#ifdef _HAVE_PROTOS
_g_render_string_non_hershey (const char *s, bool do_render, int x_justify, int y_justify)
#else
_g_render_string_non_hershey (s, do_render, x_justify, y_justify)
     const char *s;
     bool do_render;		/* whether to draw the string */
     int x_justify, y_justify;
#endif
{
  int h_just = JUST_LEFT;	/* all devices can handle left justification */
  unsigned short *codestring;
  unsigned short *cptr;
  double width = 0.0;
  double pushed_width = 0.0;	/* pushed by user */
  int current_font_index;
  /* initial values of these attributes (will be restored at end) */
  double initial_font_size;
  char *initial_font_name;
  our_font_type initial_font_type;
  /* initial and saved locations */
  double initial_position_x = _plotter->drawstate->pos.x;
  double initial_position_y = _plotter->drawstate->pos.y;
  double pushed_position_x = _plotter->drawstate->pos.x;
  double pushed_position_y = _plotter->drawstate->pos.y;
  /* misc. */
  char x_justify_c, y_justify_c;
  double x_offset, y_offset;
  double x_displacement = 1.0, overall_width = 0.0;
  double ascent, descent;
  double userdx, userdy, theta, sintheta = 0.0, costheta = 1.0;
  
  /* convert string to a codestring, including annotations */
  codestring = _controlify ((const unsigned char *)s);

  if (do_render)		/* perform needed computations; reposition */
    {
      /* compute label width in user units via a recursive call; final two
	 args here are ignored */

      overall_width = _g_render_string_non_hershey (s, false, 'c', 'c');
      
      /* compute initial offsets that must be performed due to
       justification; also displacements that must be performed after
       rendering (see above)*/
      x_justify_c = (char)x_justify;
      y_justify_c = (char)y_justify;  

      switch (x_justify_c)
	{
	case 'l': /* left justified */
	default:
	  h_just = JUST_LEFT;
	  x_offset = 0.0;
	  x_displacement = 1.0;
	  /* range [0,1] */
	  break;
	  
	case 'c': /* centered */
	  h_just = JUST_CENTER;
	  x_offset = -0.5;
	  x_displacement = 0.0;
	  /* range [-0.5,0.5] */
	  break;
	  
	case 'r': /* right justified */
	  h_just = JUST_RIGHT;
	  x_offset = -1.0;
	  x_displacement = -1.0;
	  /* range [-1,0] */
	  break;
	}

      /* need these to compute offset for vertical justification */
      ascent = _plotter->drawstate->font_ascent;
      descent = _plotter->drawstate->font_descent;
      
      switch (y_justify_c)		/* placement of label with respect
					   to y coordinate */
	{
	case 'b':			/* current point is at bottom */
	  y_offset = descent;
	  break;
	  
	case 'x':			/* current point is on baseline */
	default:
	  y_offset = 0.0;
	  break;
	  
	case 'c':			/* current point is midway between bottom, top */
	  y_offset = 0.5 * (descent - ascent);
	  break;
	  
	case 't':			/* current point is at top */
	  y_offset = - ascent;
	  break;
	}

      /* If codestring is a string in a single font, with no control codes,
	 we'll render it using native device justification, rather than
	 positioning a left-justified string by hand.  In other words if
	 right or centered justification was specified when alabel() was
	 called by the user, the string as drawn on the device will have
	 the same justification.  This is particularly important for the
	 Fig driver.  Anything else would exasperate the user, even if the
	 positioning is correct. */

      if (_plotter->have_justification && _simple_string (codestring))
	/* don't perform manual horizontal justification */
	x_offset = 0.0;
      else
	h_just = JUST_LEFT;	/* use x_offset to position by hand */
	  
      /* justification-related offsets we'll carry out */
      userdx = x_offset * overall_width;
      userdy = y_offset;
      
      /* label rotation angle in radians */
      theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
      sintheta = sin (theta);
      costheta = cos (theta);

      /* perform both horizontal and vertical offsets; after this, current
	 point will be on intended baseline of label */
      _plotter->drawstate->pos.x += costheta * userdx - sintheta * userdy;
      _plotter->drawstate->pos.y += sintheta * userdx + costheta * userdy;
    }

  /* save font name (will be restored at end) */
  initial_font_name = _plotter->drawstate->font_name;
  _plotter->drawstate->font_name = 
    (char *)_plot_xmalloc (1 + strlen (initial_font_name));
  strcpy (_plotter->drawstate->font_name, initial_font_name);

  /* save font size too */
  initial_font_size = _plotter->drawstate->font_size;

  /* also save the font type, since for fonts of type F_OTHER (e.g.,
     user-specified X Windows fonts not in our tables), switching fonts
     between substrings, e.g. to use the X Windows symbol font, may
     inconveniently switch _plotter->drawstate->font_type on us */
  initial_font_type = _plotter->drawstate->font_type;

  /* initialize current font index (font type presumably is not Hershey) */
  switch (_plotter->drawstate->font_type)
    {
    case F_POSTSCRIPT:
      current_font_index =
	(_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case F_PCL:
      current_font_index =
	(_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case F_STICK:
      current_font_index =
	(_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case F_OTHER:
      current_font_index = 1;	/* `1' just means the font we start out with */
      break;
    default:			/* unsupported font type */
      return 0.0;
    }

  /* now loop through codestring, parsing each code in succession */
  cptr = codestring;
  while (*cptr)			/* end when (unsigned short)0 is seen */
    {
      unsigned short c;
      
      c = *cptr;
      if (c & CONTROL_CODE)	
	/* parse control code; many possibilities */
	{	
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      width += SUBSCRIPT_DX * _plotter->drawstate->true_font_size;
	      if (do_render)
		{
		  _plotter->drawstate->pos.x += 
		    (costheta * SUBSCRIPT_DX - sintheta * SUBSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		  _plotter->drawstate->pos.y += 
		    (sintheta * SUBSCRIPT_DX + costheta * SUBSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		}
	      _plotter->drawstate->font_size *= SCRIPTSIZE;
	      _plotter->retrieve_font();
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      width += SUPERSCRIPT_DX * _plotter->drawstate->true_font_size;
	      if (do_render)
		{
		  _plotter->drawstate->pos.x += 
		    (costheta * SUPERSCRIPT_DX - sintheta * SUPERSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		  _plotter->drawstate->pos.y += 
		    (sintheta * SUPERSCRIPT_DX + costheta * SUPERSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		}
	      _plotter->drawstate->font_size *= SCRIPTSIZE;
	      _plotter->retrieve_font();
	      break;

	    case C_END_SUBSCRIPT:
	      width -= SUBSCRIPT_DX * _plotter->drawstate->true_font_size;
	      _plotter->drawstate->font_size /= SCRIPTSIZE;
	      _plotter->retrieve_font();
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= (costheta * SUBSCRIPT_DX 
						   - sintheta * SUBSCRIPT_DY) * _plotter->drawstate->true_font_size;
		  (_plotter->drawstate->pos).y -= (sintheta * SUBSCRIPT_DX
						   + costheta * SUBSCRIPT_DY) * _plotter->drawstate->true_font_size;
		}
	      break;
	      
	    case C_END_SUPERSCRIPT:
	      width -= SUPERSCRIPT_DX * _plotter->drawstate->true_font_size;
	      _plotter->drawstate->font_size /= SCRIPTSIZE;
	      _plotter->retrieve_font();
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= (costheta * SUPERSCRIPT_DX 
						   - sintheta * SUPERSCRIPT_DY) * _plotter->drawstate->true_font_size;
		  (_plotter->drawstate->pos).y -= (sintheta * SUPERSCRIPT_DX
						   + costheta * SUPERSCRIPT_DY) * _plotter->drawstate->true_font_size;
		}
	      break;
	      
	    case C_PUSH_LOCATION:
	      pushed_position_x = _plotter->drawstate->pos.x;
	      pushed_position_y = _plotter->drawstate->pos.y;
	      pushed_width = width;
	      break;
	      
	    case C_POP_LOCATION:
	      if (do_render)
		{
		  _plotter->drawstate->pos.x = pushed_position_x;
		  _plotter->drawstate->pos.y = pushed_position_y;
		}
	      width = pushed_width;
	      break;
	      
	    case C_RIGHT_ONE_EM:
	      if (do_render)
		{
		  _plotter->drawstate->pos.x += costheta * _plotter->drawstate->true_font_size;
		  _plotter->drawstate->pos.y += sintheta * _plotter->drawstate->true_font_size;
		}
	      width += _plotter->drawstate->true_font_size;
	      break;
	      
	    case C_RIGHT_HALF_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 2.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 2.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 2.0;
	      break;

	    case C_RIGHT_QUARTER_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 4.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 4.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 4.0;
	      break;

	    case C_RIGHT_SIXTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 6.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 6.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 6.0;
	      break;

	    case C_RIGHT_EIGHTH_EM:
	      if (do_render)
		{

		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 8.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 8.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 8.0;
	      break;

	    case C_RIGHT_TWELFTH_EM:
	      if (do_render)
		{

		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 12.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 12.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 8.0;
	      break;

	      /* Kludge: used only for \rn macro, i.e. in square roots, if
		 the current font is a PS or PCL font.  See g_cntrlify.c.
		 If the font is a Hershey font, \rn is implemented
		 differently, and for Stick fonts it isn't implemented at all.

		 Painfully, the amount of shift differs depending whether
		 this is a PS or a PCL typeface, since the `radicalex'
		 characters are quite different.  See comment in
		 g_cntrlify.c. */
	    case C_RIGHT_RADICAL_SHIFT:
	      if (do_render)
		{
		  if (_plotter->drawstate->font_type == F_PCL)
		    {
		      (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		    }
		  else
		    {
		      (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		    }
		  /* I'm going to let this serve for the PCL case; it seems to
		     work (i.e. yield more or less the correct width).  We
		     definitely don't want PCL_RADICAL_WIDTH here. */
		}
	      width += _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
	      break;

	    case C_LEFT_ONE_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size;
		}
	      
	      width -= _plotter->drawstate->true_font_size;
	      break;
	      
	    case C_LEFT_HALF_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 2.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 2.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 2.0;
	      break;

	    case C_LEFT_QUARTER_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 4.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 4.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 4.0;
	      break;

	    case C_LEFT_SIXTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 6.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 6.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 6.0;
	      break;

	    case C_LEFT_EIGHTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 8.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 8.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 8.0;
	      break;

	    case C_LEFT_TWELFTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 12.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 12.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 8.0;
	      break;

	      /* Kludge: used only for \rn macro, i.e. in square roots.
		 Painfully, the amount of shift differs depending whether
		 this is a PS or a PCL typeface, since the `radicalex'
		 characters are quite different.  See comment above, and
		 comment in g_cntrlify.c. */
	    case C_LEFT_RADICAL_SHIFT:
	      if (do_render)
		{
		  if (_plotter->drawstate->font_type == F_PCL)
		    {
		      (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		    }
		  else
		    {
		      (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		    }
		}
	      /* see comment in C_RIGHT_RADICAL_SHIFT case, above */
	      width -= _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
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
	  int new_font_index = (c >> FONT_SHIFT) & ONE_BYTE;

	  /* perform font switching if necessary */
	  if (new_font_index != current_font_index)
	    {
	      /* We check initial_font_type, not _drawstate->font_type,
		 because the latter gets trashed if (1) we start out with a
		 font of type F_OTHER, e.g. a user-specified X Windows font
		 not in our tables, and (2) we switch to the X Windows
		 Symbol font in mid-string, since that font is of type
		 F_POSTSCRIPT. */
	      switch (initial_font_type)
		{
		case F_HERSHEY:
		  free (_plotter->drawstate->font_name);
		  _plotter->drawstate->font_name =
		    (char *)_plot_xmalloc(1 + strlen (_vector_font_info[new_font_index].name));
		  strcpy (_plotter->drawstate->font_name, _vector_font_info[new_font_index].name);
		  break;
		case F_POSTSCRIPT:
		  free (_plotter->drawstate->font_name);
		  _plotter->drawstate->font_name =
		    (char *)_plot_xmalloc(1 + strlen (_ps_font_info[new_font_index].ps_name));
		  strcpy (_plotter->drawstate->font_name, _ps_font_info[new_font_index].ps_name);
		  break;
		case F_PCL:
		  free (_plotter->drawstate->font_name);
		  _plotter->drawstate->font_name =
		    (char *)_plot_xmalloc(1 + strlen (_pcl_font_info[new_font_index].ps_name));
		  strcpy (_plotter->drawstate->font_name, _pcl_font_info[new_font_index].ps_name);
		  break;
		case F_STICK:
		  free (_plotter->drawstate->font_name);
		  _plotter->drawstate->font_name =
		    (char *)_plot_xmalloc(1 + strlen (_stick_font_info[new_font_index].ps_name));
		  strcpy (_plotter->drawstate->font_name, _stick_font_info[new_font_index].ps_name);
		  break;
		case F_OTHER:
		  free (_plotter->drawstate->font_name);
		  if (new_font_index == 0) /* symbol font */
		    {
		      _plotter->drawstate->font_name =
			(char *)_plot_xmalloc(1 + strlen (SYMBOL_FONT));
		      strcpy (_plotter->drawstate->font_name, SYMBOL_FONT);
		    }
		  else
		    /* Currently, only alternative to zero (symbol font) is
                       1, i.e. restore font we started out with. */
		    {
		      _plotter->drawstate->font_name =
			(char *)_plot_xmalloc(1 + strlen (initial_font_name));
		      strcpy (_plotter->drawstate->font_name, initial_font_name);
		    }
		  break;
		default:	/* unsupported font type, shouldn't happen */
		  break;
		}

	      _plotter->retrieve_font();
	      current_font_index = new_font_index;
	    }
	  
	  /* extract substring consisting of characters in the same font */
	  sptr = s 
	    = (unsigned char *)_plot_xmalloc ((unsigned int) (4 * _codestring_len (cptr) + 1));
	  while (*cptr 
		 && (*cptr & CONTROL_CODE) == 0 
		 && ((*cptr >> FONT_SHIFT) & ONE_BYTE) == current_font_index)
	    *sptr++ = (*cptr++) & ONE_BYTE;
	  *sptr = (unsigned char)'\0';

	  /* Compute width of single-font substring in user units, add it.
	     Either render or not, as requested. */
	  width += _g_render_simple_string_non_hershey (s, do_render, h_just);
	  free (s);
	}
    }

  /* free the codestring (no memory leaks please) */
  free (codestring);

  /* restore initial font */
  free (_plotter->drawstate->font_name);
  _plotter->drawstate->font_name = initial_font_name;
  _plotter->drawstate->font_size = initial_font_size;
  _plotter->retrieve_font();
  
  if (do_render)
    {
      /* restore position to what it was before printing label */
      _plotter->drawstate->pos.x = initial_position_x;
      _plotter->drawstate->pos.y = initial_position_y;
      /* shift due to printing of label */
      _plotter->drawstate->pos.x += costheta * x_displacement * overall_width;
      _plotter->drawstate->pos.y += sintheta * x_displacement * overall_width;
    }

  return width;
}

/* Compute the width of a single-font string (escape sequences not
   recognized), and render it, if requested.  The method that is invoked
   may depend on the Plotter object type (i.e. type of display device), as
   well as on the font type.  The PS, PCL, and Stick methods appear below.

   The rendering only takes place if the do_render flag is set.  If it is
   not, the width is returned only (the h_just argument being ignored). */

static double 
#ifdef _HAVE_PROTOS
_g_render_simple_string_non_hershey (const unsigned char *s, bool do_render, int h_just)
#else
_g_render_simple_string_non_hershey (s, do_render, h_just)
     const unsigned char *s;
     bool do_render;
     int h_just;		/* horiz. justification: JUST_LEFT, etc. */
#endif
{
  switch (_plotter->drawstate->font_type)
    {
    case F_HERSHEY:
      /* Aargh.  The Hershey-specific routines do much more than is needed:
	 they handle escape sequences too, via their own controlification.
	 So we must escape all backslashes before using them. */
      {
	unsigned char *t;
	double width;
	
	t = _esc_esc_string (s);
	width = (do_render ? 
		 _g_falabel_hershey ('l', 'x', t) : 
		 _g_flabelwidth_hershey (t));
	free (t);
      }
      
    case F_POSTSCRIPT:
	return (do_render ? 
		_plotter->falabel_ps (s, h_just) : 
		_plotter->flabelwidth_ps (s));
    case F_PCL:
      return (do_render ? 
	      _plotter->falabel_pcl (s, h_just) : 
	      _plotter->flabelwidth_pcl (s));
    case F_STICK:
      return (do_render ? 
	      _plotter->falabel_stick (s, h_just) : 
	      _plotter->flabelwidth_stick (s));
    case F_OTHER:
      return (do_render ? 
	      _plotter->falabel_other (s, h_just) : 
	      _plotter->flabelwidth_other (s));
    default:			/* unsupported font type */
      return 0.0;
    }
}

/* A generic internal method that computes the width (total delta x) of a
   character string to be rendered in one of the 35 standard PS fonts, in
   user units.  The font used is the currently selected one (assumed to be
   a PS font). */

double
#ifdef _HAVE_PROTOS
_g_flabelwidth_ps (const unsigned char *s)
#else
_g_flabelwidth_ps (s)
     const unsigned char *s;
#endif
{
  int index;
  int width = 0;
  unsigned char current_char;
  int master_font_index;	/* index into master table */

  /* compute font index in master PS font table */
  master_font_index =
    (_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  for (index=0; s[index]!='\0'; index++)
    {
      current_char = (unsigned int)s[index];
      width 
	+= ((_ps_font_info[master_font_index]).width)[current_char];
    }

  return _plotter->drawstate->true_font_size * (double)width / 1000.0;
}

/* A generic internal method that computes the width (total delta x) of a
   character string to be rendered in one of the 45 standard PCL fonts, in
   user units.  The font used is the currently selected one (assumed to be
   a PCL font). */

double
#ifdef _HAVE_PROTOS
_g_flabelwidth_pcl (const unsigned char *s)
#else
_g_flabelwidth_pcl (s)
     const unsigned char *s;
#endif
{
  int index;
  int width = 0;
  unsigned char current_char;
  int master_font_index;	/* index into master table */

  /* compute font index in master PCL font table */
  master_font_index =
    (_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  for (index=0; s[index]!='\0'; index++)
    {
      current_char = (unsigned int)s[index];
      width 
	+= ((_pcl_font_info[master_font_index]).width)[current_char];
    }

  return _plotter->drawstate->true_font_size * (double)width / 1000.0;
}


/* A generic internal method that computes the width (total delta x) of a
   character string to be rendered in a stick font [i.e. device-resident HP
   font], in user units.  The font used is the currently selected one
   (assumed to be a stick font). */

/* The width tables for stick fonts (in g_fontdb.c) give character widths
   in terms of abstract raster units (the grid on which the character was
   defined).  Font size is twice the width of the abstract raster (by
   definition).  So in principle, to compute the width of a character
   string, we just need to add the character widths together, and normalize
   using the font size.

   It's more complicated than that, in part because our width tables for
   stick fonts, unlike those for PCL fonts, contain the bounding box widths
   rather than the character cell widths.  Also, for agreement with the PS
   rendering convention, we need to add a bit of leading whitespace, and a
   bit of trailing whitespace.

   We handle the string width computation in one of two ways.

   1. Modern HP-GL/2.  Much as for the PCL fonts, the true width
   (character cell width) of a character equals 

   	offset + bounding box width + offset

   In fact, offset is independent of character; it depends only on the
   font.  So the string width we compute for a string consisting of n
   characters is:

      offset + bb width #1 + offset 
    + offset + bb width #2 + offset 
    + ...
    + offset + bb width #n + offset

   The first and last offsets in this formula provide the leading and
   trailing bits of whitespace.

   2. Pre-HP-GL/2 (i.e. HPGL_VERSION="1.5").  There is automatic kerning,
   according to HP's device-resident spacing tables (the spacing tables are
   in g_fontd2.c).  The string width we return is:

    offset + bb width #1 + spacing(1,2) + bb width #2 + spacing(2,3) 
           + ... + spacing(n-1,n) + bb width #n + offset

   where spacing(1,2) is the spacing between characters 1 and 2, etc.

   The basic reference for HP's pre-HP-GL/2 kerning scheme is "Firmware
   Determines Plotter Personality", by L. W. Hennessee, A. K. Frankel,
   M. A. Overton, and R. B. Smith, Hewlett-Packard Journal, Nov. 1981,
   pp. 16-25.  Every character belongs to a `row class' and a `column
   class', i.e., `right edge class' and `left edge class'.  Any spacing
   table is indexed by row class and column class.

   [What HP later did in HP-GL/2 is apparently a degenerate case of this
   setup, with all the inter-character spacings changed to 2*offset.]

   A couple of additional comments on the pre-HP-GL/2 case:

   Comment A.  The width of the space character (ASCII SP) is 3/2 times as
   large in pre-HP-GL/2 as it is in HP-GL/2.  In HP-GL/2, it's 0.5 times
   the font size, but in pre-HP-GL/2, it's 0.75 times the font size.  That
   sounds like a major difference, but the use of kerning more or less
   compensates for it.  See comment in code below.

   Comment B.  Our ANK fonts consist of a lower half encoded according to
   JIS ASCII, and an upper half encoded according to the half-width
   Katakana encoding.  These two halves are different HP character sets and
   use different spacing tables, since their abstract raster widths differ
   (42 and 45, respectively).  HP's convention is apparently that if,
   between character k and character k+1, there's a switch between spacing
   tables and spacing(k,k+1) can't be computed via lookup, then

   	bb width #k + spacing(k,k+1) + bb width #(k+1)

   should be replaced by

   	width_of_space_character + bb width #(k+1)

   That's the way we do it.
*/

double
#ifdef _HAVE_PROTOS
_g_flabelwidth_stick (const unsigned char *s)
#else
_g_flabelwidth_stick (s)
     const unsigned char *s;
#endif
{
  double width = 0.0;		/* normalized units (font size = 1.0) */
  int index;
  int master_font_index;	/* index into master font array */

  /* compute font index in master table of device-resident HP fonts */
  master_font_index =
    (_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  if (_plotter->hpgl_version < 2)
    /* presumably HPGL_VERSION="1.5" i.e. HP7550A; have device-resident
       kerning, so we compute inter-character spacing from spacing tables
       in g_fontd2.c, which we know match the device-resident tables */
    {
      const struct stick_kerning_table_struct *ktable_lower, *ktable_upper;
      const struct stick_spacing_table_struct *stable_lower, *stable_upper;
      const short *lower_spacing, *upper_spacing;	/* spacing tables */
      int lower_rows, lower_cols, upper_rows, upper_cols; /* table sizes */
      const char *lower_char_to_row, *lower_char_to_col; /* map char to pos */
      const char *upper_char_to_row, *upper_char_to_col; /* map char to pos */
      bool halves_use_different_tables; /* upper/lower spacing tables differ?*/

      /* kerning table and spacing table structs, for each font half */
      ktable_lower = &(_stick_kerning_tables[_stick_font_info[master_font_index].kerning_table_lower]);
      ktable_upper = &(_stick_kerning_tables[_stick_font_info[master_font_index].kerning_table_upper]);
      stable_lower = &(_stick_spacing_tables[ktable_lower->spacing_table]);
      stable_upper = &(_stick_spacing_tables[ktable_upper->spacing_table]);
      
      /* do font halves use different spacing tables (e.g. ANK fonts)? */
      halves_use_different_tables 
	= (stable_lower != stable_upper ? true : false);
      
      /* numbers of rows and columns in each of the two spacing tables */
      lower_rows = stable_lower->rows;
      lower_cols = stable_lower->cols;  
      upper_rows = stable_upper->rows;
      upper_cols = stable_upper->cols;  
      
      /* arrays (size 128), mapping character to row/column of spacing table */
      lower_char_to_row = ktable_lower->row;
      lower_char_to_col = ktable_lower->col;  
      upper_char_to_row = ktable_upper->row;
      upper_char_to_col = ktable_upper->col;  
      
      /* spacing tables for each half of the font */
      lower_spacing = stable_lower->kerns;
      upper_spacing = stable_upper->kerns;  
      
      /* our convention: add an initial bit of whitespace (an `offset'), to
	 make the stick font rendering agree with the PS rendering
	 convention */
      width		
	+= (((double)(_stick_font_info[master_font_index].offset))
	    /(2 * _stick_font_info[master_font_index].raster_width_lower));
      
      /* loop through chars in label */
      for (index=0; s[index]!='\0'; index++)
	{
	  unsigned char c, d;
	  
	  c = (unsigned int)s[index];

	  if (c < 0x80)
	    /* lower half */
	    {
	      double spacefactor, char_width;

	      /* Our width tables in g_fontd2.c are suitable for HP-GL/2,
		 not pre-HP-GL/2.  Major difference is that in pre-HP-GL/2,
		 width of space character is 3/2 times larger, e.g. in the
		 Arc font it is 42 abstract raster units rather than 28.
		 (This difference is partly compensated for by pre-HP-GL/2
		 having kerning, unlike HP-GL/2.)  */
	      if (c == ' ')
		spacefactor = 1.5;
	      else
		spacefactor = 1.0;

	      /* add width of char */
	      char_width
		= (((double)(_stick_font_info[master_font_index].width[c]))
		   * spacefactor
		   /(2 * _stick_font_info[master_font_index].raster_width_lower));
	      width += char_width;

	      if ((d = (unsigned int)s[index+1]) != '\0')
		/* current char is not final char in string, so add spacing
                   between it and the next char */
		{
		  int row, col;
		  int spacing;
		  
		  /* compute row class for current character, i.e., its
		     `right edge class' */
		  row = lower_char_to_row[c];

		  /* compute and add spacing; if we switch from lower to
		     upper half here, and upper half uses a different
		     spacing table, just replace width of c by width of ` '
		     (see explanation above) */
		  if (d < 0x80)
		    {
		      col = lower_char_to_col[d];
		      spacing = lower_spacing[row * lower_cols + col];
		    }
		  else if (!halves_use_different_tables)
		    {
		      col = upper_char_to_col[d - 0x80];
		      spacing = lower_spacing[row * lower_cols + col];
		    }
		  else if (c == ' ' || (d == ' ' + 0x80))
		    /* space characters have no kerning */
		    spacing = 0;
		  else	
		    /* c -> ` ', see above. */
		    spacing = 
		      - IROUND(spacefactor * _stick_font_info[master_font_index].width[c])
		      + IROUND(1.5 * _stick_font_info[master_font_index].width[' ']);

		  width		
		    += ((double)spacing)
		      /(2 * _stick_font_info[master_font_index].raster_width_lower);
		}
	    }
	  else
	    /* upper half */
	    {
	      double spacefactor, char_width;

	      if (c == ' ' + 0x80) /* i.e. `unbreakable SP' */
		spacefactor = 1.5;
	      else
		spacefactor = 1.0;

	      /* add width of char */
	      char_width		
		= (((double)(_stick_font_info[master_font_index].width[c]))
		   /(2 * _stick_font_info[master_font_index].raster_width_upper));
	      width += char_width;

	      if ((d = (unsigned int)s[index+1]) != '\0')
		/* current char is not final char in string, so add spacing
                   between it and the next char */
		{
		  int row, col;
		  int spacing;
		  
		  /* compute row class for current character, i.e., its
		     `right edge class' */
		  row = upper_char_to_row[c - 0x80];

		  /* compute and add spacing; if we switch from upper to
		     lower half here, and lower half uses a different
		     spacing table, just replace width of c by width of ` '
		     (see explanation above) */
		  if (d >= 0x80)
		    {
		      col = upper_char_to_col[d - 0x80];
		      spacing = upper_spacing[row * upper_cols + col];
		    }
		  else if (!halves_use_different_tables)
		    {
		      col = lower_char_to_col[d];
		      spacing = upper_spacing[row * upper_cols + col];
		    }
		  else if ((c == ' ' + 0x80) || d == ' ')
		    /* space characters have no kerning */
		    spacing = 0;
		  else
		    /* c -> ` ', see above. */
		    spacing = 
		      - IROUND(spacefactor * _stick_font_info[master_font_index].width[c])
		      + IROUND(1.5 * _stick_font_info[master_font_index].width[' ']);

		  width		
		    += ((double)spacing)
		      /(2 * _stick_font_info[master_font_index].raster_width_upper);
		}
	    }
	}

      /* our convention: add a trailing bit of whitespace (an `offset'), to
	 make the stick font rendering agree with the PS rendering
	 convention */
      width		
	+= (((double)(_stick_font_info[master_font_index].offset))
	    /(2 * _stick_font_info[master_font_index].raster_width_lower));
    }
  else
    /* HPGL_VERSION="2", i.e. HP-GL/2; no device-resident kerning.  We use
       a fixed offset between each pair of characters, which is the way
       HP-GL/2 devices do it.  We also use this offset as the width of the
       `bit of whitespace' that we add at beginning and end of label. */
    {
      /* loop through chars in label */
      for (index=0; s[index]!='\0'; index++)
	{
	  unsigned char c;

	  c = (unsigned int)s[index];

#if 0
	  /* kludge around HP's convention for centered marker symbols
	     (poor fellows ain't got no width a-tall) */
	  if (IS_MATH_FONT(master_font_index) && IS_CENTERED_SYMBOL(c))
	    continue;
#endif
	  if (c < 0x80)
	    /* lower half */
	    {
	      width		
		+= (((double)(_stick_font_info[master_font_index].offset))
		    /(2 * _stick_font_info[master_font_index].raster_width_lower));
	      width		
		+= (((double)(_stick_font_info[master_font_index].width[c]))
		    /(2 * _stick_font_info[master_font_index].raster_width_lower));
	      width		
		+= (((double)(_stick_font_info[master_font_index].offset))
		    /(2 * _stick_font_info[master_font_index].raster_width_lower));
	    }
	  else
	    /* upper half */
	    {
	      width		
		+= (((double)(_stick_font_info[master_font_index].offset))
		    /(2 * _stick_font_info[master_font_index].raster_width_upper));
	      width	
		+= (((double)(_stick_font_info[master_font_index].width[c]))
		    /(2 * _stick_font_info[master_font_index].raster_width_upper));
	      width		
		+= (((double)(_stick_font_info[master_font_index].offset))
		    /(2 * _stick_font_info[master_font_index].raster_width_upper));
	    }
	}
    }

  /* normalize: use font size to convert width to user units */
  return _plotter->drawstate->true_font_size * (double)width;
}


/* test whether a controlified string is simple in the sense that it
   consists of characters in the same font, and no control codes */
static bool
#ifdef _HAVE_PROTOS
_simple_string (const unsigned short *codestring)
#else
_simple_string (codestring)
     const unsigned short *codestring;
#endif
{
  const unsigned short *cptr = codestring;
  unsigned short c, d;
  int font_index;

  if (*codestring == 0)
    return true;
  c = *codestring;
   if (c & CONTROL_CODE)
    return false;
  font_index = (c >> FONT_SHIFT) & ONE_BYTE;
  while ((d = *cptr++) != 0)
    {
      int local_font_index;

      if (d & CONTROL_CODE)
	return false;
      local_font_index = (d >> FONT_SHIFT) & ONE_BYTE;      
      if (local_font_index != font_index)
	return false;
    }
  return true;
}

/* escape all backslashes in a string; the returned string is allocated on
   the heap and can be freed. */
static unsigned char *
#ifdef _HAVE_PROTOS
_esc_esc_string (const unsigned char *s)
#else
_esc_esc_string (s)
     unsigned const char *s;
#endif
{
  const unsigned char *sptr;
  unsigned char *t, *tptr;

  t = (unsigned char *)_plot_xmalloc (2 * strlen ((char *)s) + 1);
  sptr = s;
  tptr = t;
  while (*sptr)
    {
      *tptr++ = *sptr;
      if (*sptr == '\\')
	*tptr++ = *sptr;
      sptr++;
    }
  *tptr = '\0';

  return t;
}
