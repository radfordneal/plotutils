/* This file contains a low-level method for adjusting the font of an HP-GL
   plotter to agree with an HPGLPlotter's notion of what the graphics
   cursor position should be, prior to plotting a label using PCL fonts.

   This has an effect only if (1) HP-GL/2 rather than HP-GL is being
   emitted, and the current font is a PCL or Stick font, or (2) HP-GL is
   being used, and the current font is a Stick font.  HP-GL, as opposed to
   HP-GL/2, does not support PCL fonts. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* Shearing factor for oblique fonts, new_x = x + SHEAR * y  */

#define SHEAR (2.0/7.0)

/* forward references */
static bool _hpgl_maybe_update_font __P((void));
static bool _hpgl2_maybe_update_font __P((void));

void
#ifdef _HAVE_PROTOS
_h_set_font (void)
#else
_h_set_font ()
#endif
{
  bool font_changed = false;
  bool oblique;
  double cos_slant = 1.0, sin_slant = 0.0;
  double dx, dy, perpdx, perpdy;
  double len, perplen, tan_slant;
  double relative_label_run, relative_label_rise;
  double theta, sintheta, costheta;
  int master_font_index;
  
  /* sanity check, should be unnecessary */
  if (_plotter->hpgl_version == 0 /* i.e. generic HP-GL */
      || (_plotter->drawstate->font_type != F_PCL
	  && _plotter->drawstate->font_type != F_STICK))
    return;

  if (_plotter->drawstate->font_type == F_STICK)
    /* check whether obliquing of this font is called for */
    {
      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      oblique = _stick_font_info[master_font_index].obliquing;
    }
  else
    oblique = false;

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  costheta = cos (theta);
  sintheta = sin (theta);

  /* compute lengths in device frame of two vectors that are (in the user
     frame) directed along the label and perpendicular to the label, with
     length (in the user frame) equal to the font size */
  dx = _plotter->drawstate->true_font_size * XDV(costheta,sintheta);
  dy = _plotter->drawstate->true_font_size * YDV(costheta,sintheta);
  len = sqrt (dx * dx + dy * dy);

  perpdx = _plotter->drawstate->true_font_size * XDV(-sintheta,costheta);
  perpdy = _plotter->drawstate->true_font_size * YDV(-sintheta,costheta);

  /* actually, even in the user frame the second vector may be obliqued */
  perpdx += (oblique ? SHEAR : 0.0) * dx;
  perpdy += (oblique ? SHEAR : 0.0) * dy;  

  perplen = sqrt (perpdx * perpdx + perpdy * perpdy);

  /* Compute rise and run, relative to scaling points. (Either or both can
     be negative; overall normalization, e.g. the `100', is irrelevant.  We
     include the `100' to express them as percentages.) */
  relative_label_run =  100 * dx / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
  relative_label_rise = 100 * dy / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
  if (relative_label_run != 0.0 || relative_label_rise != 0.0)
    /* (will always be true except when the font size is so small
       there's really no point in printing the label) */
    {
      /* update device-frame label rotation angle if needed */
      if (_plotter->relative_label_run != relative_label_run
	  || _plotter->relative_label_rise != relative_label_rise)
	{    
	  sprintf (_plotter->page->point, "DR%.3f,%.3f;",
		   relative_label_run, relative_label_rise);
	  _update_buffer (_plotter->page);
	  _plotter->relative_label_run = relative_label_run;
	  _plotter->relative_label_rise = relative_label_rise;
	}
    }

  /* emit command to change font, if needed */
  if (_plotter->hpgl_version == 2)
    font_changed = _hpgl2_maybe_update_font();
  else				/* 1, i.e. "1.5", i.e. HP7550A */
    font_changed = _hpgl_maybe_update_font();

  /* compute character slant (device frame) */
  if (len == 0.0 || perplen == 0.0) /* a bad situation */
    tan_slant = 0.0;
  else
    {
      sin_slant = (dx * perpdx + dy * perpdy) / (len * perplen);
      cos_slant = sqrt (1 - sin_slant * sin_slant);
      tan_slant = sin_slant / cos_slant;
    }

  /* Compute nominal horizontal and vertical character sizes as percentages
     of the horizontal and vertical distances between scaling points P1 and
     P2, and specify them with the SR instruction.  

     The 0.5 and 0.7 = 1.4 * 0.5 factors are undocumented HP magic.  The
     arguments of the SR instruction should apparently be 0.5 times the
     font size, and 0.7 times the font size.  This convention must have
     been introduced by HP to adapt the SR instruction, which dates back to
     fixed-width plotter fonts (Stick fonts) to modern outline fonts.
     Fixed-width plotter fonts did not have a font size in the modern
     sense; they had a character width and a character height.  (The former
     being the width of the character proper, which occupied the left 2/3
     of a character cell, and the latter being what we would nowadays call
     a cap height.)

     The convention probably arose because Stick fonts look best if the
     aspect ratio is 1.4 (= 0.7/0.5), i.e. if the `character height' is 1.4
     times the `character width'.  I do not know where the 0.5 came from.
     Possibly back in stick font days, the font size was defined to be
     4/3 times the width of a character cell, or equivalently the width
     of a character cell was chosen to be 3/4 times the nominal font size.
     This would make the maximum character width 
     (2/3)x(3/4) = (1/2) times the font size. */

  {
    int orientation = _plotter->drawstate->transform.nonreflection ? 1 : -1;
    double fractional_char_width, fractional_char_height;    
    double relative_char_width, relative_char_height;

    fractional_char_width = 0.5;
    fractional_char_height = 1.4 * 0.5;

    relative_char_width = fractional_char_width * 100 * len / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
    relative_char_height = 
      fractional_char_height * 100 * orientation * cos_slant * perplen / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
    
    /* emit SR instruction only if font was changed or if current
       size was wrong */
    if (font_changed || 
	(relative_char_width != _plotter->relative_char_width
	 || relative_char_height != _plotter->relative_char_height))
      {
	sprintf (_plotter->page->point, "SR%.3f,%.3f;", 
		 relative_char_width, relative_char_height);
	_update_buffer (_plotter->page);
	_plotter->relative_char_width = relative_char_width;
	_plotter->relative_char_height = relative_char_height;
      }
  }

  /* update slant angle if necessary */
  if (tan_slant != _plotter->char_slant_tangent)
    {
      sprintf (_plotter->page->point, "SL%.3f;", tan_slant);
      _update_buffer (_plotter->page);
      _plotter->char_slant_tangent = tan_slant;
    }

  return;
}

/* If needed, emit a font-change command, HP-GL/2 style.  Return value
   indicates whether font was changed. */

static bool
#ifdef _HAVE_PROTOS
_hpgl2_maybe_update_font (void)
#else
_hpgl2_maybe_update_font ()
#endif
{
  bool font_change = false;
  int master_font_index;
  int symbol_set, spacing, posture, stroke_weight, typeface;

  /* PCL and Stick fonts are handled separately here only because the font
     information for them is stored in two different tables in
     g_fontdb.c. */

  if (_plotter->drawstate->font_type == F_PCL)
    {
      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      /* compute character sizes (width / height) for SR or SI instruction */
      
      /* #1: symbol set */
      symbol_set = _pcl_font_info[master_font_index].pcl_symbol_set;
      /* #2: spacing */
      spacing = _pcl_font_info[master_font_index].pcl_spacing;
      /* #3, #4 are pitch and height (we use defaults) */
      /* #5: posture */
      posture = _pcl_font_info[master_font_index].pcl_posture;
      /* #6: stroke weight */
      stroke_weight = _pcl_font_info[master_font_index].pcl_stroke_weight;
      /* #7: typeface */
      typeface = _pcl_font_info[master_font_index].pcl_typeface;  
    }
  else			/* F_STICK */
    {
      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      /* compute character sizes (width / height) for SR or SI instruction */
      
      /* #1: symbol set */
      symbol_set = _stick_font_info[master_font_index].pcl_symbol_set;
      /* #2: spacing */
      spacing = _stick_font_info[master_font_index].pcl_spacing;
      /* #3, #4 are pitch and height (we use defaults) */
      /* #5: posture */
      posture = _stick_font_info[master_font_index].pcl_posture;
      /* #6: stroke weight */
      stroke_weight = _stick_font_info[master_font_index].pcl_stroke_weight;
      /* #7: typeface */
      typeface = _stick_font_info[master_font_index].pcl_typeface;  
    }
  
  if (symbol_set != _plotter->pcl_symbol_set
      || spacing != _plotter->pcl_spacing
      || posture != _plotter->pcl_posture
      || stroke_weight != _plotter->pcl_stroke_weight
      || typeface != _plotter->pcl_typeface)
    font_change = true;
  
  if (font_change)
    {
      if (spacing == FIXED_SPACING)
				/* fixed-width font */
	sprintf (_plotter->page->point, 
		 /* #4 (nominal point size) not needed but included anyway */
		 "SD1,%d,2,%d,3,%.3f,4,%.3f,5,%d,6,%d,7,%d;",
		 symbol_set, spacing, 
		 (double)NOMINAL_CHARS_PER_INCH, (double)NOMINAL_POINT_SIZE, 
		 posture, stroke_weight, typeface);
      else			/* variable-width font */
	sprintf (_plotter->page->point, 
		 /* #3 (nominal chars per inch) not needed but included anyway*/
		 "SD1,%d,2,%d,3,%.3f,4,%.3f,5,%d,6,%d,7,%d;",
		 symbol_set, spacing, 
		 (double)NOMINAL_CHARS_PER_INCH, (double)NOMINAL_POINT_SIZE, 
		 posture, stroke_weight, typeface);
      _update_buffer (_plotter->page);

      _plotter->pcl_symbol_set = symbol_set;
      _plotter->pcl_spacing = spacing;
      _plotter->pcl_posture = posture;
      _plotter->pcl_stroke_weight = stroke_weight;
      _plotter->pcl_typeface = typeface;
    }

  return font_change;		/* was font changed? */
}

/* If needed, emit an old-style (pre-HP-GL/2) `CS' font-change command.
   Return value indicates whether font was changed. */

static bool
#ifdef _HAVE_PROTOS
_hpgl_maybe_update_font (void)
#else
_hpgl_maybe_update_font ()
#endif
{
  bool font_change = false;
  int hp_charset_lower, hp_charset_upper, master_font_index;

  /* compute index of font in master table of fonts, in g_fontdb.c */
  master_font_index =
    (_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  
  /* determine HP character set numbers (old style, pre-HP-GL/2) */
  hp_charset_lower = _stick_font_info[master_font_index].hp_charset_lower;
  hp_charset_upper = _stick_font_info[master_font_index].hp_charset_upper;

  /* select charset for lower half of font */
  if (hp_charset_lower != _plotter->hp_charset_lower)
    {
      sprintf (_plotter->page->point, "CS%d;", hp_charset_lower);
      _update_buffer (_plotter->page);
      _plotter->hp_charset_lower = hp_charset_lower;
      font_change = true;
    }

  /* select charset for upper half, if we have a genuine one (a negative
     value for the upper charset is our way of flagging that this is a
     7-bit font; see h_alab_pcl.c) */
  if (hp_charset_upper >= 0 
      && hp_charset_upper != _plotter->hp_charset_upper)
    {
      sprintf (_plotter->page->point, "CA%d;", hp_charset_upper);
      _update_buffer (_plotter->page);
      _plotter->hp_charset_upper = hp_charset_upper;
      font_change = true;
    }

  return font_change;
}
