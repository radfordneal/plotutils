/* This is the low-level falabel_pcl() method, which renders a single-font
   label in a PCL font.  This version is for HP-GL and PCL Plotters.  It is
   now used for PS and Stick fonts too.

   Pre-HP-GL/2 devices supported only Stick fonts.  Only a few modern
   high-end devices (e.g. LaserJet 4000 series laser printers) support PS
   fonts in PCL mode.  PS fonts are supported by HP-GL/2 and PCL Plotters
   if the `--enable-ps-fonts-in-pcl' option is specified at configure time.

   Novel features of this driver include (1) the rightward shift that all
   fonts need, (2) the re-encoding that Stick fonts need, (3) the
   compensation for the kerning that was automatically used when
   pre-HP-GL/2 devices rendered Stick fonts, and (4) the re-encoding that
   ISO-Latin-1 PCL fonts need, due to HP's idiosyncratic definition of
   ISO-Latin-1 ("ECMA-96 Latin 1").

   1. HP-GL rendering of a string is displaced leftward, relative to PS
   rendering, by an amount equal to the distance between the bounding box
   left edge and the left edge (`first ink') for the first character.  This
   is so that the first ink will be put on the page right where we start
   rendering the string.  This convention dates back to pen plotter days.

   The offset[] arrays in g_fontdb.c hold the information that we need to
   undo this leftward shift by a compensating initial rightward shift.
   After rendering the string, we undo the shift. 

   2. Stick fonts are available only in HP's Roman-8 encoding.  So we
   need to remap them, if they are to be ISO-Latin-1 fonts.  There are
   a few characters missing, but we do our best. 

   3. In pre-HP-GL/2, unlike HP-GL/2, Stick fonts were automatically
   kerned.  We compensate for this in g_alabel.c, if HPGL_VERSION="1.5", by
   using the spacing tables in g_fontd2.c when computing label widths.  The
   inclusion of kerning in the label width computation affects the
   horizontal positioning of the label, if it is centered or
   right-justifified rather than left-justified.

   4. PCL fonts (and the PS fonts available in PCL mode on a few high-end
   devices) in principle support ISO-Latin-1 encoding, natively.  However,
   HP interprets ISO-Latin-1 in an idiosyncratic way.  For example,
   left-quote and right-quote show up as accents, and tilde shows up as a
   tilde accent.  For this reason, for ISO-Latin-1 PCL fonts we use HP's
   Roman-8 encoding for the lower half, and HP's ISO-Latin-1 encoding for
   the upper half. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "h_stick.h"

/* for switching to upper half of font charset */
#define SHIFT_OUT 14		/* i.e. ASCII 0x0e, i.e. ^N */
#define SHIFT_IN 15		/* i.e. ASCII 0x0f, i.e. ^O */

/* for DFA */
typedef enum { LOWER_HALF, UPPER_HALF } state_type;

/* kludge, see comment in code */
#define HP_ROMAN_8_MINUS_CHAR 0366

/* Kludges to handle the zero-width marker symbols in our ArcMath and
   StickSymbol fonts; also zero-width overbar.
   8 AND 17 ARE EFFECTIVELY HARDCODED IN THE FONT TABLE IN g_fontd2.c !! */
#define ARCSYMBOL 8
#define STICKSYMBOL 17
#define IS_SYMBOL_FONT(fontnum) ((fontnum) == ARCSYMBOL || (fontnum) == STICKSYMBOL)
#define IS_CENTERED_SYMBOL(c) (((c) >= 'A' && (c) <= 'O') || (c) == 'e')

double
#ifdef _HAVE_PROTOS
_h_falabel_pcl (const unsigned char *s, int h_just)
#else
_h_falabel_pcl (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  bool created_temp_string = false;
  bool reencode_iso_as_roman8 = false;
  double hp_offset;
  double theta, costheta, sintheta;
  double width;
  int master_font_index;
  unsigned char *t;
  unsigned char instruction_buf[4];
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  if (h_just != JUST_LEFT)
    {
      _plotter->warning ("ignoring request to use non-default justification for a label");
      return 0.0;
    }

  /* sanity check, should be unnecessary */
  if (_plotter->hpgl_version == 0 /* generic HP-GL, no non-Hershey fonts */
      || _plotter->drawstate->font_type == F_HERSHEY)
    return 0.0;

  /* compute index of font in master table in g_fontdb.c; also width
     of string in user units */
  switch (_plotter->drawstate->font_type)
    {
    case F_PCL:
    default:
      master_font_index =
	(_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      width = _plotter->flabelwidth_pcl (s);
      break;
    case F_POSTSCRIPT:
      master_font_index =
	(_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      width = _plotter->flabelwidth_ps (s);
      break;
    case F_STICK:
      master_font_index =
	(_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      width = _plotter->flabelwidth_stick (s);
      break;
    }

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  switch (_plotter->drawstate->font_type)
    {
    case F_PCL:
    default:
      if (_pcl_font_info[master_font_index].pcl_symbol_set == PCL_ROMAN_8
	  && _pcl_font_info[master_font_index].iso8859_1)
	/* An ISO-Latin-1 PCL font, must use HP's Roman-8 for lower half
	   and HP's Latin-1 for upper half.  With one exception: we map the
	   ASCII minus character `-' to 0366.  This is a kludge, needed to
	   get a character whose width matches the width in the AFM files
	   that HP distributes. */
	{
	  state_type dfa_state = LOWER_HALF;
	  unsigned const char *sptr = s;
	  unsigned char *tptr;

	  /* temp string for rewritten label */
	  t = (unsigned char *)_plot_xmalloc (3 * strlen ((const char *)s) + 1);
	  tptr = t;
	  created_temp_string = true;

	  /* SHIFT_IN switches to standard charset, SHIFT_OUT to alternative*/
	  while (*sptr)
	    {
	      unsigned char c;

	      c = *sptr++;
	      if (c < 0x80)
		/* lower half of font, use standard font (HP Roman-8) */
		{
		  if (c == '-')	/* kludge, map to a char in upper half */
		    c = HP_ROMAN_8_MINUS_CHAR;
		  if (dfa_state == UPPER_HALF)
		    {
		      *tptr++ = SHIFT_IN;
		      dfa_state = LOWER_HALF;
		    }
		  *tptr++ = c;
		}
	      else
		/* upper half of font, use alt. font (HP ECMA-96 Latin-1) */
		{
		  if (dfa_state == LOWER_HALF)
		    {
		      *tptr++ = SHIFT_OUT;
		      dfa_state = UPPER_HALF;
		    }
		  *tptr++ = c;
		}
	    }
	  
	  if (dfa_state == UPPER_HALF)
	    *tptr++ = SHIFT_IN;
	  *tptr = '\0';	/* end of rewritten label */
	}
      else
	/* a non-ISO-Latin-1 PCL font, no need for reencoding */
	t = (unsigned char *)s;
      break;
    case F_POSTSCRIPT:
      /* no need for reencoding (HP's encoding is good enough) */
      t = (unsigned char *)s;
      break;
    case F_STICK:
      if (_stick_font_info[master_font_index].pcl_symbol_set == PCL_ROMAN_8
	  && _stick_font_info[master_font_index].iso8859_1)
	/* stick font uses HP's Roman-8 encoding for its upper half, so
           must reencode ISO-Latin-1 as Roman-8 */
	reencode_iso_as_roman8 = true;

      if (_plotter->hpgl_version == 1)
	/* HP-GL version is "1.5", i.e. HP7550A; will reencode upper half
           as 7-bit */
	{
	  bool bogus_upper_half = false;
	  state_type dfa_state = LOWER_HALF;
	  unsigned const char *sptr = s;
	  unsigned char *tptr;

	  /* Check whether font is meant to be a 7-bit font with no upper
	     half; if so, we'll ignore all 8-bit characters.  We set the
	     charset number for the upper half to be -1 if this is to be
	     the case (see table in g_fontdb.c). */
	  if (_stick_font_info[master_font_index].hp_charset_upper < 0)
	    bogus_upper_half = true;

	  /* temp string for rewritten label */
	  t = (unsigned char *)_plot_xmalloc (3 * strlen ((const char *)s) + 1);
	  tptr = t;
	  created_temp_string = true;

	  /* do 7-bit reencoding, first mapping to Roman-8 if necessary */
	  /* SHIFT_IN switches to standard charset, SHIFT_OUT to alternative*/
	  while (*sptr)
	    {
	      unsigned char c;

	      c = *sptr++;
	      if (c >= 0x80 && reencode_iso_as_roman8)
		/* reencode upper half via lookup table in h_stick.h */
		c = _iso_to_roman8[c - 0x80];

	      if (c < 0x80)
		/* lower half of font, pass through */
		{
		  if (dfa_state == UPPER_HALF)
		    {
		      *tptr++ = SHIFT_IN;
		      dfa_state = LOWER_HALF;
		    }
		  *tptr++ = c;
		}
	      else
		/* upper half of font, move to lower half */
		if (bogus_upper_half == false)
		  {
		    if (dfa_state == LOWER_HALF)
		      {
			*tptr++ = SHIFT_OUT;
			dfa_state = UPPER_HALF;
		      }
		    *tptr++ = c - 0x80;
		  }
	    }
	  
	  if (dfa_state == UPPER_HALF)
	    *tptr++ = SHIFT_IN;
	  *tptr = '\0';	/* end of rewritten label */
	}
      else
	/* HP-GL version is "2", i.e. HP-GL/2, so the only Stick fonts
	   we have are 8-bit ones; no need for 7-bit reencoding or DFA.
	   May still need to map ISO-Latin-1 to Roman-8, though. */
	{
	  unsigned const char *sptr = s;
	  unsigned char *tptr;
	
	  t = (unsigned char *)_plot_xmalloc (strlen ((const char *)s) + 1);
	  tptr = t;
	  created_temp_string = true;
	  while (*sptr)
	    {
	      if (*sptr < 0x80)
		*tptr++ = *sptr++;
	      else
		{
		  if (reencode_iso_as_roman8)
		    /* reencode upper half via lookup table in h_stick.h */
		    *tptr++ = _iso_to_roman8[(*sptr++) - 0x80];
		  else
		    *tptr++ = *sptr++;
		}
	    }
	  *tptr = '\0';		/* end of rewritten label */
	}
      break;
    }
  
  /* compute abovementioned HP-style rightward shift; depends on `offset'
     for first character in label */
  switch (_plotter->drawstate->font_type)
    {
    case F_PCL:
    default:
      /* per-character offset expressed in units where font size = 1000 */
      hp_offset = _pcl_font_info[master_font_index].offset[*((unsigned char *)s)] / 1000.0;
      break;
    case F_POSTSCRIPT:
      /* per-character offset expressed in units where font size = 1000 */
      hp_offset = _ps_font_info[master_font_index].offset[*((unsigned char *)s)] / 1000.0;
      break;
    case F_STICK:
      /* Offset expressed in HP's abstract raster units, need to divide by
	 what the font size equals in raster units.  
	 (Font size = 2 * raster width, by definition.) */

      /* For HP7550A fonts that we've defined in such a way that the raster
	 width differs between lower and upper halves, not sure what to do
	 here.  In particular ArcANK has JIS-ASCII encoding for lower half,
	 with raster width 42, and half-width Katakana encoding for upper
	 half, with raster width 45.  For now, just use the raster width
	 for the lower half. */

#if 0
      /* kludge around possibility that first character in string is a
	 centered symbol in the ArcSymbol or StickSymbol fonts */
      if (IS_SYMBOL_FONT(master_font_index) 
	  && IS_CENTERED_SYMBOL(*((unsigned char *)s)))
	hp_offset = 0.0;
      else
#endif
	hp_offset = (((double)(_stick_font_info[master_font_index].offset)) /
		     (2.0 * _stick_font_info[master_font_index].raster_width_lower));
      break;
    }

  /* do the rightward shift */
  _plotter->drawstate->pos.x += 
    costheta * _plotter->drawstate->true_font_size * hp_offset;
  _plotter->drawstate->pos.y += 
    sintheta * _plotter->drawstate->true_font_size * hp_offset;

  /* sync font and pen position */
  _plotter->set_font();
  _plotter->set_position();

  /* Sync pen color.  This may set the _plotter->bad_pen flag (if optimal
     pen is #0 [white] and we're not allowed to use pen #0 to draw with).
     So we test _plotter->bad_pen before drawing the label (see below). */
  _plotter->set_pen_color ();

  if (t[0] != '\0' /* i.e. label nonempty */
      && _plotter->bad_pen == false)
    /* output the label via an `LB' instruction, including label
       terminator; don't use sprintf to avoid having to escape % and \ */
    {
      strcpy (_plotter->page->point, "LB");
      _update_buffer (_plotter->page);
      strcpy (_plotter->page->point, (const char *)t);
      _update_buffer (_plotter->page);      
      instruction_buf[0] = (unsigned char)3; /* ^C = default label terminator*/
      instruction_buf[1] = ';';
      instruction_buf[2] = '\0';
      strcpy (_plotter->page->point, (const char *)instruction_buf);
      _update_buffer (_plotter->page);

      /* where is the plotter pen now located?? we don't know, exactly */
      _plotter->position_is_unknown = true;
    }

   if (created_temp_string)
     /* created a temp string, so free it */
     free (t);

  /* Undo HP's rightward shift */

  _plotter->drawstate->pos.x -=
    costheta * _plotter->drawstate->true_font_size * hp_offset;
  _plotter->drawstate->pos.y -= 
    sintheta * _plotter->drawstate->true_font_size * hp_offset;

  /* reposition after drawing label */
  _plotter->drawstate->pos.x += costheta * width;
  _plotter->drawstate->pos.y += sintheta * width;

  return width;
}
