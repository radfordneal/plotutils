/* _controlify() converts a "label" (a character string), which may contain
   troff-like escape sequences, into a string of unsigned shorts.  The
   possible troff-like escape sequences are listed in control.h.

   This conversion is to facilitate rendering.  _controlify() is called by
   alabel(), and the converted label is rendered by _alabel_standard(),
   _alabel_stroke(), or _alabel_device(), depending on what sort of font is
   currently selected.  See alabel.c, and also labelwidth.c (_controlify()
   is called by labelwidth() too).

   If the currently selected font is a font with ISO-Latin-1 encoding, the
   valid escape sequences include escape sequences for the non-ASCII
   ISO-Latin-1 characters.  Also allowed are such escape sequences as \f0,
   \f1, \f2, \f3, \f4, etc., which switch among the various fonts in the
   current typeface:

   	\f1	Switch to font #1, basic
	\f2	Switch to font #2, italic
	\f3	Switch to font #3, bold
	\f4	Switch to font #4, bold italic
	\f0	Switch to font #0, symbol (including Greek characters)
   
   All typefaces include at least two fonts: fonts #0 and #1.  Some may
   include more than the above five.  Each unsigned short in the converted
   string is really an annotated character: the low byte is the character,
   and the high byte is the font number.

   Besides font annotations, the controlified string may include control
   codes (unsigned shorts with particular bit patterns in their high
   bytes), which are produced by the escape sequences:

        \sp  start superscript
        \ep  end superscript

        \sb  start subscript
        \eb  end subscript

        \mk  mark location
        \rt  return to mark 
	     [useful e.g. for underlining, and filling square roots]

    There are also control codes for horizontal shifts.  \r1, \r2, \r4,
    \r6, \r8 will produce control codes that will shift right by 1 em, 1/2
    em, 1/4 em, 1/6 em, 1/8 em, respectively.  \l1, \l2, \l4, \l6, \l8 are
    similar.

    The string of unsigned shorts, which is returned, is allocated with
    malloc and may be freed later. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_control.h"
#include "g_cntrlify.h"
#include "g_jis.h"

#define NUM_OCCIDENTAL_VECTOR_GLYPHS 4400 /*should agree with val in her_glyphs.c */
#define NUM_ORIENTAL_VECTOR_GLYPHS 5500	/*should agree with value in her_glyphs.c */

/* UGS `undefined character' symbol (several horizontal strokes), i.e.
   location in extended occidental Hershey glyph array */
#define UNDE 4023

unsigned short *
#ifdef _HAVE_PROTOS
_controlify (const unsigned char *src)
#else
_controlify (src)
     const unsigned char *src;
#endif
{     
  unsigned short *dest;
  unsigned char c, d;
  unsigned char esc[3];
  int j = 0;
  int raw_fontnum, raw_symbol_fontnum;
  unsigned short fontword, symbol_fontword;
  
  /* note: string length can grow by a factor of 6, because a single
     printable character can be mapped to a sequence of unsigned shorts, of
     length up to 6 (see comment below) */
  dest = (unsigned short *)_plot_xmalloc ((6 * strlen ((char *)src) + 1) * sizeof(unsigned short));
  
  /* Determine initial number of font, as index into low-level table in
     fontdb.c, and the initial value for the shifted `font word' which
     we'll OR with each character; also same, for associated symbol font.
     Can be updated by \f0, \f1, etc. */
  switch (_plotter->drawstate->font_type)
    {
    case F_POSTSCRIPT:
    default:
      raw_fontnum = _ps_typeface_info[_plotter->drawstate->typeface_index].fonts[_plotter->drawstate->font_index];
      raw_symbol_fontnum = _ps_typeface_info[_plotter->drawstate->typeface_index].fonts[0];
      break;
    case F_PCL:
      raw_fontnum = _pcl_typeface_info[_plotter->drawstate->typeface_index].fonts[_plotter->drawstate->font_index];
      raw_symbol_fontnum = _pcl_typeface_info[_plotter->drawstate->typeface_index].fonts[0];
      break;
    case F_HERSHEY:
      raw_fontnum = _vector_typeface_info[_plotter->drawstate->typeface_index].fonts[_plotter->drawstate->font_index];
      raw_symbol_fontnum = _vector_typeface_info[_plotter->drawstate->typeface_index].fonts[0];
      break;
    case F_OTHER:
      /* no real font table in this case; by convention font #1 internally
         means the current device-specific font and #0 is an associated
         symbol font */
      raw_fontnum = 1;
      raw_symbol_fontnum = 0;
      break;
    }
  fontword = ((unsigned short)raw_fontnum) << FONT_SHIFT;
  symbol_fontword = ((unsigned short)raw_symbol_fontnum) << FONT_SHIFT;

  while (*src != (unsigned char)'\0')
    {
      /* If EUC, check first for high bit and process two-byte characters
	 separately.  This approach is awkward (we duplicate a lot of code
	 here, which appears elsewhere below).  FIXME. */

      if ((raw_fontnum == HERSHEY_EUC_FONT) 
	  && (*src & 0x80) && (*(src + 1) & 0x80))
	{
	  unsigned char jis_row = *src & ~(0x80);
	  unsigned char jis_col = *(src + 1) & ~(0x80);

	  if (GOOD_JIS_INDEX(jis_row, jis_col))
	    {
	      int jis_glyphindex = 256 * jis_row + jis_col;
	  
	      if (jis_glyphindex >= BEGINNING_OF_KANJI)
		/* in Kanji range, so check if we have it */
		{
#ifndef NO_KANJI
		  const struct kanjipair *kanji = _builtin_kanji_glyphs;
		  bool matched = false;
		  
		  while (kanji->jis != 0)
		    {
		      if (jis_glyphindex == kanji->jis)
			{
			  matched = true;
			  break;
			}
		      kanji++;
		    }
		  if (matched)
		    {
		      dest[j++] = RAW_ORIENTAL_HERSHEY_GLYPH | (kanji->nelson);
		      src += 2;
		      continue;	/* back to top of while loop */
		    }
		  else		/* a Kanji we don't have */
		    {
		      /* render as standard `undefined character' glyph */
		      dest[j++] = RAW_HERSHEY_GLYPH | UNDE;
		      src += 2;
		      continue;	/* back to top of while loop */
		    }
#endif /* not NO_KANJI */
		}
	      else
		/* not in Kanji range, so look for it in char table */
		{
		  const struct jis_entry *char_mapping = _builtin_jis_chars;
		  bool matched = false;
		  
		  while (char_mapping->jis != 0)
		    {
		      if (jis_glyphindex == char_mapping->jis)
			{
			  matched = true;
			  break;
			}
		      char_mapping++;
		    }
		  if (matched)
		    /* the entry in the character table maps the JIS
		       character to a character (in 0..255 range) in
		       one of the fonts in the master table in fontdb.c */
		    {
		      int fontnum = char_mapping->font;
		      unsigned short charnum = char_mapping->charnum;
		      
		      if (charnum & RAW_HERSHEY_GLYPH)
			/* a raw Hershey glyph, not in any font */
			dest[j++] = RAW_HERSHEY_GLYPH | charnum;
		      else
			/* a character in one of the fonts in fontdb.c */
			dest[j++] = (((unsigned short)fontnum) << FONT_SHIFT) | charnum;
		      src += 2;
		      continue; /* back to top of while loop */
		    }
		  else	/* a character we don't have */
		    {
		      /* render as standard `undefined character' glyph */
		      dest[j++] = RAW_HERSHEY_GLYPH | UNDE;
		      src += 2;
		      continue;	/* back to top of while loop */
		    }
		}
	    }
	  else
	    /* JIS index is OOB */
	    {
	      src += 2;
	      continue;		/* back to top of while loop */
	    }
	}

      /* if current font is Hershey, first try to match each ligature
         pattern (no ligatures supported in non-Hershey fonts) */
      if (_plotter->drawstate->font_type == F_HERSHEY)
	{
	  int i;
	  bool matched = false;
	  
	  for (i = 0; i < NUM_LIGATURES; i++) 
	    if ((_ligature_tbl[i].font == raw_fontnum)
		&& (strncmp ((char *)src, _ligature_tbl[i].from, 
			     strlen (_ligature_tbl[i].from)) == 0))
	      {
		matched = true;
		break;
	      }
	  
	  if (matched)
	    {
	      dest[j++] = fontword | (unsigned short)_ligature_tbl[i].byte;
	      src += strlen (_ligature_tbl[i].from);
	      continue;		/* back to top of while loop */
	    }
	}

      c = *(src++);		/* no ligature, so get single new char */
      if (c != (unsigned char)'\\') /* ordinary char, may pass through */
	{
	  /* if current font is an ISO-Latin-1 Hershey font ... */
	  if ((_plotter->drawstate->font_type == F_HERSHEY)
	      && _plotter->drawstate->font_is_iso8859)
	    {
	      int i;
	      bool matched = false;

	      /* check if this is a `raised' ISO-Latin-1 character */
	      for (i = 0; i < NUM_RAISED_CHARS; i++) 
		if (c == _raised_char_tbl[i].from)
		  {
		    matched = true;
		    break;
		  }
	      if (matched)	/* it's a raised character */
		{
		  /* map to string of unsigned shorts, length 3 or 6:
		     `begin superscript' control code, [`mark'
		     control code,] replacement char, [`return'
		     control code, underline,] `end superscript' */
		  dest[j++] = 
		    (unsigned short) (CONTROL_CODE | C_BEGIN_SUPERSCRIPT);
		  if (_raised_char_tbl[i].underscored) /* also underline */
		    {
		      dest[j++] = 
			(unsigned short) (CONTROL_CODE | C_PUSH_LOCATION);
		      dest[j++] = 
			fontword | (unsigned short)_raised_char_tbl[i].to;
		      dest[j++] = 
			(unsigned short) (CONTROL_CODE | C_POP_LOCATION);
		      /* select appropriate HersheySymbol font */
		      dest[j++] = 
			symbol_fontword | (unsigned short)VECTOR_SYMBOL_FONT_UNDERSCORE;
		    }
		  else	/* just print raised char, no underline */
		    dest[j++] = 
		      fontword | (unsigned short)_raised_char_tbl[i].to;
		  
		  dest[j++] = 
		    (unsigned short) (CONTROL_CODE | C_END_SUPERSCRIPT);
		  
		  continue; /* back to top of while loop */
		}

	      /* also check if this char should be deligatured */
	      for (i = 0; i < NUM_DELIGATURED_CHARS; i++) 
		if (c == _deligature_char_tbl[i].from)
		  {
		    matched = true;
		    break;
		  }
	      if (matched)
		{
		  if (_deligature_char_tbl[i].except_font != raw_fontnum)
		    {
		      dest[j++] = fontword 
			| (unsigned short)_deligature_char_tbl[i].to[0];
		      dest[j++] = fontword 
			| (unsigned short)_deligature_char_tbl[i].to[1];
		      continue;	/* back to top of while loop */
		    }
		}
	    }
	  
	  /* didn't do anything special, so just pass the character thru */
	  dest[j++] = fontword | (unsigned short)c; 
	  continue;		/* back to top of while loop */
	}
      else			/* character is a backslash */
	{
	  int i;
      
	  c = *(src++);		/* grab next character */
	  if (c == (unsigned char)'\0')	/* ASCII NUL ? */
	    {
	      dest[j++] = fontword | (unsigned short)'\\';
	      break;		/* string terminated with a backslash */
	    }
	  
 	  if (c == (unsigned char)'\\')
	    {
	      dest[j++] = fontword | (unsigned short)'\\';
	      dest[j++] = fontword | (unsigned short)'\\';
	      continue;		/* saw \\, leave as is */
	    }

	  d = *(src++);
 	  if (d == (unsigned char)'\0')
	    {
	      dest[j++] = fontword | (unsigned short)'\\';
	      dest[j++] = fontword | (unsigned short)c;
	      break;		/* string terminated with \c */
	    }

	  esc[0] = c;
	  esc[1] = d;
	  esc[2] = (unsigned char)'\0';	/* have an escape sequence */
	  
	  /* is this an escape seq. (e.g. \#H0001) for a raw Hershey glyph? */
	  if (_plotter->drawstate->font_type == F_HERSHEY
	      && esc[0] == '#' && esc[1] == 'H'
	      && src[0] >= '0' && src[0] <= '9'
	      && src[1] >= '0' && src[1] <= '9'
	      && src[2] >= '0' && src[2] <= '9'
	      && src[3] >= '0' && src[3] <= '9')
	    {
	      int glyphindex;

	      glyphindex = (src[3] - '0') + 10 * (src[2] - '0')
		+ 100 * (src[1] - '0') + 1000 * (src[0] - '0');
	      if (glyphindex < NUM_OCCIDENTAL_VECTOR_GLYPHS)
		{
		  dest[j++] = RAW_HERSHEY_GLYPH | glyphindex;
		  src += 4;
		  continue;
		}
	    }

#ifndef NO_KANJI
	  /* is this an escape seq. (e.g. \#N0001) for a raw Japanese
             Hershey glyph (Kanji), as numbered in Nelson's dictionary? */
	  if (_plotter->drawstate->font_type == F_HERSHEY
	      && esc[0] == '#' && esc[1] == 'N'
	      && src[0] >= '0' && src[0] <= '9'
	      && src[1] >= '0' && src[1] <= '9'
	      && src[2] >= '0' && src[2] <= '9'
	      && src[3] >= '0' && src[3] <= '9')
	    {
	      int glyphindex;

	      glyphindex = (src[3] - '0') + 10 * (src[2] - '0')
		+ 100 * (src[1] - '0') + 1000 * (src[0] - '0');
	      if (glyphindex < NUM_ORIENTAL_VECTOR_GLYPHS)
		{
		  dest[j++] = RAW_ORIENTAL_HERSHEY_GLYPH | glyphindex;
		  src += 4;
		  continue;
		}
	    }
#endif /* not NO_KANJI */

	  /* is this an escape seq. (e.g. \#J0001) for a raw Japanese
             Hershey glyph (JIS numbering, in hex)? */
	  if (_plotter->drawstate->font_type == F_HERSHEY
	      && esc[0] == '#' && esc[1] == 'J'
	      && ((src[0] >= '0' && src[0] <= '9')
		  || (src[0] >= 'a' && src[0] <= 'f')
		  || (src[0] >= 'A' && src[0] <= 'F'))
	      && ((src[1] >= '0' && src[1] <= '9')
		  || (src[1] >= 'a' && src[1] <= 'f')
		  || (src[1] >= 'A' && src[1] <= 'F'))
	      && ((src[2] >= '0' && src[2] <= '9')
		  || (src[2] >= 'a' && src[2] <= 'f')
		  || (src[2] >= 'A' && src[2] <= 'F'))
	      && ((src[3] >= '0' && src[3] <= '9')
		  || (src[3] >= 'a' && src[3] <= 'f')
		  || (src[3] >= 'A' && src[3] <= 'F')))
	    {
	      int jis_glyphindex;
	      int i, hexnum[4];
	      int jis_row, jis_col;
	      
	      for (i = 0; i < 4; i++)
		if (src[i] >= 'a' && src[i] <= 'f')
		  hexnum[i] = 10 + src[i] - 'a';
		else if (src[i] >= 'A' && src[i] <= 'F')
		  hexnum[i] = 10 + src[i] - 'A';
		else /* a decimal digit */
		  hexnum[i] = src[i] - '0';

	      jis_glyphindex = (hexnum[3] + 16 * hexnum[2]
				+ 256 * hexnum[1] + 4096 * hexnum[0]);
	      jis_row = hexnum[1] + 16 * hexnum[0];
	      jis_col = hexnum[3] + 16 * hexnum[2];

	      if (GOOD_JIS_INDEX(jis_row, jis_col))
		{
		  if (jis_glyphindex >= BEGINNING_OF_KANJI)
		    /* in Kanji range, so check if we have it */
		    {
#ifndef NO_KANJI
		      const struct kanjipair *kanji = _builtin_kanji_glyphs;
		      bool matched = false;
		      
		      while (kanji->jis != 0)
			{
			  if (jis_glyphindex == kanji->jis)
			    {
			      matched = true;
			      break;
			    }
			  kanji++;
			}
		      if (matched)
			{
			  dest[j++] = RAW_ORIENTAL_HERSHEY_GLYPH | (kanji->nelson);
			  src += 4;
			  continue;	/* back to top of while loop */
			}
		      else		/* a Kanji we don't have */
			{
			  /* render as standard `undefined character' glyph */
			  dest[j++] = RAW_HERSHEY_GLYPH | UNDE;
			  src += 4;
			  continue;	/* back to top of while loop */
			}
#endif /* not NO_KANJI */
		    }
		  else
		    /* not in Kanji range, so look for it in char table */
		    {
		      const struct jis_entry *char_mapping = _builtin_jis_chars;
		      bool matched = false;
		      
		      while (char_mapping->jis != 0)
			{
			  if (jis_glyphindex == char_mapping->jis)
			    {
			      matched = true;
			      break;
			    }
			  char_mapping++;
			}
		      if (matched)
			/* the entry in the character table maps the JIS
			   character to a character (in 0..255 range) in
			   one of the fonts in the master table in fontdb.c */
			{
			  int fontnum = char_mapping->font;
			  unsigned short charnum = char_mapping->charnum;
			  
			  if (charnum & RAW_HERSHEY_GLYPH)
			    /* a raw Hershey glyph, not in any font */
			    dest[j++] = RAW_HERSHEY_GLYPH | charnum;
			  else
			    /* a character in one of the fonts in fontdb.c */
			    dest[j++] = (((unsigned short)fontnum) << FONT_SHIFT) | charnum;
			  src += 4;
			  continue; /* back to top of while loop */
			}
		      else	/* a character we don't have */
			{
			  /* render as standard `undefined character' glyph */
			  dest[j++] = RAW_HERSHEY_GLYPH | UNDE;
			  src += 4;
			  continue;	/* back to top of while loop */
			}
		    }
		}
	    }

	  {
	    bool matched = false;

	    /* is this an escape seq. for a control code? */
	    for (i = 0; i < NUM_CONTROLS; i++) 
	      if (strcmp ((char *)esc, _control_tbl[i]) == 0)
		{
		  matched = true;
		  break;
		}
	    if (matched)		/* it's a control code */
	      {
		dest[j++] = CONTROL_CODE | i;
		continue;	/* back to top of while loop */
	      }
	  }

	  /* if an ISO-Latin-1 Hershey font, is this an escape sequence for
	     an `8-bit' (non-ASCII) char, which due to nonexistence should
	     be deligatured? */
	  if ((_plotter->drawstate->font_type == F_HERSHEY)
	      && _plotter->drawstate->font_is_iso8859)
	    {
	      int i;
	      bool matched = false;
	     
	      for (i = 0; i < NUM_DELIGATURED_ESCAPES; i++)
		if (strcmp ((char *)esc, _deligature_escape_tbl[i].from) == 0)
		  {
		    matched = true;
		    break;
		  }
	      if (matched)
		{
		  if (_deligature_escape_tbl[i].except_font != raw_fontnum)
		    {
		      dest[j++] = fontword 
			| (unsigned short)_deligature_escape_tbl[i].to[0];
		      dest[j++] = fontword 
			| (unsigned short)_deligature_escape_tbl[i].to[1];
		      continue;
		    }
		}
	    }

	  /* no matter whether font is a PS font or a Hershey font, is this
             an escape seq. for an `8-bit' (non-ASCII) ISO8859-1 char? */
	  if (_plotter->drawstate->font_is_iso8859)
	    {
	      bool matched = false;

	      for (i = 0; i < NUM_ISO_ESCAPES; i++) 
		if (strcmp ((char *)esc, _iso_escape_tbl[i].string) == 0)
		  {
		    matched = true;
		    break;
		  }
	      if (matched)	/* it's an `8-bit' ISO8859-1 character */
		{
		  /* certain such characters are drawn in the Hershey fonts
                     as superscripts */
		  if (_plotter->drawstate->font_type == F_HERSHEY)
		    {
		      int k;
		      bool matched2 = false;
		      
		      /* check if this is a `raised' ISO-Latin-1 character */
		      for (k = 0; k < NUM_RAISED_CHARS; k++) 
			if (_iso_escape_tbl[i].byte == _raised_char_tbl[k].from)
			  {
			    matched2 = true;
			    break;
			  }
		      if (matched2)	/* it's a raised character */
			{
			  /* map to string of unsigned shorts, length 3 or 6:
			     `begin superscript' control code, [`mark'
			     control code,] replacement char, [`return'
			     control code, underline,] `end superscript' */
			  dest[j++] = 
			    (unsigned short) (CONTROL_CODE | C_BEGIN_SUPERSCRIPT);
			  if (_raised_char_tbl[k].underscored) /* also underline */
			    {
			      dest[j++] = 
				(unsigned short) (CONTROL_CODE | C_PUSH_LOCATION);
			      dest[j++] = 
				fontword | (unsigned short)_raised_char_tbl[k].to;
			      dest[j++] = 
				(unsigned short) (CONTROL_CODE | C_POP_LOCATION);
			      /* select appropriate HersheySymbol font */
			      dest[j++] = 
				symbol_fontword | (unsigned short)VECTOR_SYMBOL_FONT_UNDERSCORE;
			    }
			  else	/* just print raised char, no underline */
			    dest[j++] = 
			      fontword | (unsigned short)_raised_char_tbl[k].to;

			  dest[j++] = 
			    (unsigned short) (CONTROL_CODE | C_END_SUPERSCRIPT);

			  continue; /* back to top of while loop */
			}
		    }

		  /* won't draw this char as a superscript; just pass thru */
		  dest[j++] = fontword | (unsigned short)(_iso_escape_tbl[i].byte);
		  continue;	/* back to top of while loop */
		}
	    }

	  /* is this an escape seq. for a `special' (non-ISO, non-Symbol)
	     Hershey character? */
	  if (_plotter->drawstate->font_type == F_HERSHEY)
	    {
	      bool matched = false;

	      for (i = 0; i < NUM_SPECIAL_ESCAPES; i++) 
		if (strcmp ((char *)esc, _special_escape_tbl[i].string) == 0)
		  {
		    matched = true;
		    break;
		  }
	      if (matched)	/* it's a special character */
		{
		  /* "\s-" is special; yields character in current font */
		  if (_special_escape_tbl[i].byte == FINAL_LOWERCASE_S)
		  dest[j++] = 
		    fontword | (unsigned short)(_special_escape_tbl[i].byte);
		  else
		  /* we select symbol font of typeface, in which we've
		     stored all other special characters */
		    dest[j++] = symbol_fontword | (unsigned short)(_special_escape_tbl[i].byte);
		  continue;	/* back to top of while loop */
		}
	    }

	  {
	    bool matched = false;
	    
	    /* is this an escape seq. for a char in the symbol font? */
	    for (i = 0; i < NUM_SYMBOL_ESCAPES; i++) 
	      if (strcmp (_symbol_escape_tbl[i].string, "NO_ABBREV") != 0
		  && strcmp ((char *)esc, _symbol_escape_tbl[i].string) == 0)
		{
		  matched = true;
		  break;
		}
	    if (matched)	/* it's a character in the symbol font */
	      {
		/* select symbol font by OR'ing in the symbol fontword */
		dest[j++] = symbol_fontword | (unsigned short)(_symbol_escape_tbl[i].byte);
		continue;	/* back to top of while loop */
	      }
	  }

	  /* Gross kludge.  In the PS and PCL fonts we handle "\rn" by
	     mapping it into (1) a left shift, (2) the `radicalex'
	     character in the Symbol font, and (3) a right shift.  Shift
	     distances are taken from the bbox of the radicalex char, and
	     are slightly larger than 0.5 em. */
	  if ((_plotter->drawstate->font_type == F_POSTSCRIPT 
	       || _plotter->drawstate->font_type == F_PCL)
	      && (strcmp ((char *)esc, "rn") == 0))
	    {
	      dest[j++] 
		= (unsigned short)(CONTROL_CODE | C_LEFT_RADICAL_SHIFT);
	      dest[j++] 
		= symbol_fontword | (unsigned short)RADICALEX; /* symbol font */
	      dest[j++] 
		= (unsigned short)(CONTROL_CODE | C_RIGHT_RADICAL_SHIFT);
	      continue;
	    }

	  /* attempt to parse as a font-change command, i.e. as one of the
	     macros \f0, \f1, \f2, etc., unless a device-specific font is
	     being used (we have no table of such, so we don't allow
	     user-specified shifting among them). */
	  if (_plotter->drawstate->font_type != F_OTHER)
	    if (esc[0] == 'f' && esc[1] >= '0' && esc[1] <= '9')
	      {
		int new_font_index = esc[1] - '0';

		switch (_plotter->drawstate->font_type)
		  {
		  case F_HERSHEY:
		    if ((new_font_index >= _vector_typeface_info[_plotter->drawstate->typeface_index].numfonts)
			|| new_font_index < 0)
		      new_font_index = 1; /* OOB -> use default font */
		    raw_fontnum = _vector_typeface_info[_plotter->drawstate->typeface_index].fonts[new_font_index];
		    break;
		  case F_PCL:
		    if ((new_font_index >= _pcl_typeface_info[_plotter->drawstate->typeface_index].numfonts)
			|| new_font_index < 0)
		      new_font_index = 1; /* OOB -> use default font */
		    raw_fontnum = _pcl_typeface_info[_plotter->drawstate->typeface_index].fonts[new_font_index];
		    break;
		  case F_POSTSCRIPT:
		  default:
		    if ((new_font_index >= _ps_typeface_info[_plotter->drawstate->typeface_index].numfonts)
			|| new_font_index < 0)
		      new_font_index = 1; /* OOB -> use default font */
		    raw_fontnum = _ps_typeface_info[_plotter->drawstate->typeface_index].fonts[new_font_index];
		    break;
		  }

		fontword = ((unsigned short)raw_fontnum) << FONT_SHIFT;

		continue;	/* back to top of while loop */
	      }

	  /* couldn't match; unknown escape seq., so pass through unchanged */
	  dest[j++] = fontword | (unsigned short)'\\';
	  dest[j++] = fontword | (unsigned short)c;
	  dest[j++] = fontword | (unsigned short)d;
	}
    }

  dest[j] = (unsigned short)'\0';   /* terminate string */

  return dest;
}

int 
#ifdef _HAVE_PROTOS
_codestring_len (const unsigned short *codestring)
#else
_codestring_len (codestring)
     const unsigned short *codestring;
#endif
{
  int i = 0;

  while (*codestring)
    {
      i++;
      codestring++;
    }
  
  return i;
}
