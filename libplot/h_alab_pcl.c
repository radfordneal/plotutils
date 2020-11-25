/* This is the low-level falabel_pcl() method, which prints a single-font
   label in a PCL or Stick font.  This version is for HP-GL Plotters.

   Novel features are (1) the rightward shift that PCL fonts need, (2) the
   reencoding that Stick fonts need, (3) the rightward shift that Stick
   fonts need (a difficult matter, esp. for variable-width Stick fonts).

   HP-GL/2 rendering of a string is displaced leftward, relative to PS
   rendering, by an amount equal to the distance between the bounding box
   left edge and the left edge (`first ink') for the first character.  This
   is so that the first ink will be put on the page right where we start
   rendering the string.  This convention apparently goes back to HP's
   first pen plotters.

   The offset[] arrays in g_fontdb.c hold the information that we need to
   undo this leftward shift by a compensating initial rightward shift.
   After rendering the string, we undo the shift. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "h_stick.h"

/* for HP-GL/2, where we use printable chararacters as label terminators */
#define MIN_TERMINATOR 0x21
#define MAX_TERMINATOR 0xfe

/* for switching to upper half of font charset, in pre-HP-GL/2 */
#define SHIFT_OUT 14		/* i.e. ASCII 0x0e, i.e. ^N */
#define SHIFT_IN 15		/* i.e. ASCII 0x0f, i.e. ^O */

/* for DFA */
typedef enum { LOWER_HALF, UPPER_HALF } state_type;

/* forward references */
static unsigned char _label_terminator __P((const unsigned char *s));

double
#ifdef _HAVE_PROTOS
_h_falabel_pcl (const unsigned char *s, int h_just)
#else
_h_falabel_pcl (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  bool bad_label = false;
  bool stick_font = false;
  double hp_offset;
  double theta, costheta, sintheta;
  double width;
  int master_font_index;
  unsigned char *t;
  unsigned char terminator;
  unsigned char instruction_buf[4];
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  if (h_just != JUST_LEFT)
    {
      _plotter->warning ("ignoring request to use non-default justification for a label");
      return 0.0;
    }

  /* sanity check */
  if (_plotter->hpgl_version == 0 /* i.e. generic HP-GL */
      || (_plotter->drawstate->font_type != F_PCL
	  && _plotter->drawstate->font_type != F_STICK))
    return 0.0;

  /* a Stick font rather than a PCL font? */
  stick_font = (_plotter->drawstate->font_type == F_STICK ? true : false);

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* compute index of font in appropriate master table of fonts in g_fontdb.c */
  if (stick_font)
    master_font_index =
      (_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  else			/* PCL font */
    master_font_index =
      (_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* compute width of string in user units */
  if (stick_font)
    width = _plotter->flabelwidth_stick (s);
  else
    width = _plotter->flabelwidth_pcl (s);

  if (stick_font)
    /* Stick font rather than PCL font, may reencode */
    {
      bool reencode_iso_as_roman8 = false;

      if (_stick_font_info[master_font_index].pcl_symbol_set == PCL_ROMAN_8
	  && _stick_font_info[master_font_index].iso8859_1)
	/* stick font uses HP's Roman-8 encoding for its upper half, should
           reencode ISO-Latin-1 as Roman-8 */
	reencode_iso_as_roman8 = true;

      if (_plotter->hpgl_version == 1) 
	/* HP-GL version is "1.5", i.e. HP7550A; will reencode upper half
           as 7-bit */
	{
	  bool bogus_upper_half = false;
	  state_type dfa_state = LOWER_HALF;
	  unsigned const char *sptr = s;
	  unsigned char *tptr;

	  /* check whether font is meant to be a 7-bit font with no upper
	     half; if so, we'll ignore all 8-bit characters */
	  if (_stick_font_info[master_font_index].hp_charset_upper < 0)
	    bogus_upper_half = true;

	  /* temp string for rewritten label */
	  t = (unsigned char *)_plot_xmalloc (3 * strlen ((const char *)s) + 1);
	  tptr = t;

	  /* do 7-bit reencoding, with mapping to Roman-8 if necessary */
	  /* SHIFT_IN switches to standard charset, SHIFT_OUT to alternative */
	  while (*sptr)
	    {
	      unsigned char c;

	      c = *sptr++;
	      if (c >= 0x80 && reencode_iso_as_roman8)
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
		/* upper half of font, reencode (and move to lower half) */
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
	   we have are 8-bit ones; no need for 7-bit reencoding or DFA */
	{
	  unsigned const char *sptr = s;
	  unsigned char *tptr;
	
	  t = (unsigned char *)_plot_xmalloc (strlen ((const char *)s) + 1);
	  tptr = t;
	  while (*sptr)
	    {
	      if (*sptr < 0x80)
		*tptr++ = *sptr++;
	      else
		{
		  if (reencode_iso_as_roman8)
		    /* remap upper half via lookup table in h_stick.h */
		    *tptr++ = _iso_to_roman8[(*sptr++) - 0x80];
		  else
		    *tptr++ = *sptr++;
		}
	    }
	  *tptr = '\0';		/* end of rewritten label */
	}
    }
  else
    /* PCL font: no re-encoding needed, so don't rewrite label */
    t = (unsigned char *)s;
  
  /* Do abovementioned HP-style rightward shift */

  if (stick_font)
    /* Offset expressed in HP's abstract raster units, need to divide by
       what the font size equals in raster units.  
       (Font size = 2 * raster width, by definition.) */
    {
      /* For fonts in which the raster width differs between lower and
	 upper halves, not sure what to do here.  E.g., ArcANK has
	 JIS-ASCII encoding for lower half, with raster width 42, and
	 half-width Katakana encoding for upper half, with raster width 45.
         For now, just use the raster width for the lower half. */
      hp_offset = (((double)(_stick_font_info[master_font_index].offset)) /
		   (2.0 * _stick_font_info[master_font_index].raster_width_lower));
    }
  else
    /* PCL font */
    /* per-character offset expressed in units where font size = 1000 */
    hp_offset = _pcl_font_info[master_font_index].offset[t[0]] / 1000.0;

  _plotter->drawstate->pos.x += 
    costheta * _plotter->drawstate->true_font_size * hp_offset;
  _plotter->drawstate->pos.y += 
    sintheta * _plotter->drawstate->true_font_size * hp_offset;

  /* sync font and pen position */
  _plotter->set_font();
  _plotter->set_position();

  /* Sync pen color.  This may set the _plotter->bad_pen flag (if optimal
     pen is #0 and we're not allowed to use pen #0 to draw with).  So we
     test _plotter->bad_pen before drawing the label (see below). */
  _plotter->set_pen_color ();

  if (_plotter->hpgl_version == 2)
    /* Choose a printable char as label terminator.  For pre-HP-GL/2
       (e.g. HP7550A), we always use the default ^C terminator (set in
       h_defplot.c).  We don't want to use a printable character as
       terminator in that case because (aargh) until HP-GL/2, such
       terminators were printed! */
    {
      terminator = _label_terminator (t);
      if (t == (unsigned char)0)
	/* can't choose one, so won't print label */
	{
	  bad_label = true;
	  _plotter->warning ("label contains too many distinct characters to be drawn");
	}
      else
	/* if needed, select terminator via HP-GL/2 `DT' instruction; don't
	   use sprintf to avoid having to escape % and \ */
	if (terminator != _plotter->label_terminator)
	  {
	    strcpy (_plotter->page->point, "DT");
	    _update_buffer (_plotter->page);
	    instruction_buf[0] = terminator;
	    instruction_buf[1] = ';';
	    instruction_buf[2] = '\0';
	    strcpy (_plotter->page->point, (const char *)instruction_buf);
	    _update_buffer (_plotter->page);
	    _plotter->label_terminator = terminator;
	  }
    }

  if (!bad_label 
      && t[0] != '\0' /* i.e. label good and nonempty */
      && _plotter->bad_pen == false)
    /* output the label via an `LB' instruction, including terminator;
       don't use sprintf to avoid having to escape % and \ */
    {
      strcpy (_plotter->page->point, "LB");
      _update_buffer (_plotter->page);
      strcpy (_plotter->page->point, (const char *)t);
      _update_buffer (_plotter->page);      
      instruction_buf[0] = _plotter->label_terminator;
      instruction_buf[1] = ';';
      instruction_buf[2] = '\0';
      strcpy (_plotter->page->point, (const char *)instruction_buf);
      _update_buffer (_plotter->page);

      /* where is the plotter pen now located?? we don't know, exactly */
      _plotter->position_is_unknown = true;
    }

  if (stick_font)
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

/* Compute a character for use as terminator.  We use this only when
   producing HP-GL/2 output, to generate a printable character (we try to
   keep control characters out of the output stream in the HP-GL/2 case). */

static unsigned char
#ifdef _HAVE_PROTOS
_label_terminator (const unsigned char *s)
#else
_label_terminator (s)
     const unsigned char *s;	/* label we'll be rendering */
#endif
{
  unsigned char i;

  for (i = MIN_TERMINATOR; i <= MAX_TERMINATOR; i++)
    {
      const unsigned char *sptr;
      bool suitable;

      sptr = s;
      suitable = true;
      while (*sptr)
	if (*sptr++ == i)
	  {
	    suitable = false;
	    break;		/* unsuitable */
	  }
      if (suitable)
	return i;
    }
  
  /* label contains every possible character, won't be able to render it */
  return (unsigned char)0;
}
