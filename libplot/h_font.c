/* This file contains a low-level method for adjusting the font of an HP-GL
   plotter to agree with an HPGLPlotter's notion of what the graphics
   cursor position should be, prior to plotting a label using PCL fonts.

   This has an effect only if (1) HP-GL/2 rather than HP-GL is being
   emitted, and (2) the current font is a PCL font.  HP-GL does not support
   PCL fonts. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_h_set_font (void)
#else
_h_set_font ()
#endif
{
  int master_font_index;
  int symbol_set, spacing, posture, stroke_weight, typeface;
  double relative_label_run, relative_label_rise;
  bool need_new_font = false, need_new_size = false;
  double dx, dy, perpdx, perpdy;
  double theta, sintheta, costheta;
  double len, perplen, tan_slant;
  double cos_slant = 1.0, sin_slant = 0.0;
  
  /* sanity check */
  if (_plotter->hpgl_version < 2 
      || _plotter->drawstate->font_type != F_PCL)
    return;

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  costheta = cos (theta);
  sintheta = sin (theta);

  /* compute lengths in device frame of two vectors that are (in the user
     frame) directed along the label and perpendicular to the label, with
     length (in the user frame) equal to the font size */
  dx = _plotter->drawstate->font_size * XDV(costheta,sintheta);
  dy = _plotter->drawstate->font_size * YDV(costheta,sintheta);
  len = sqrt (dx * dx + dy * dy);
  perpdx = _plotter->drawstate->font_size * XDV(-sintheta,costheta);
  perpdy = _plotter->drawstate->font_size * YDV(-sintheta,costheta);
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
	  sprintf (_plotter->outbuf.current, "DR%.3f,%.3f;",
		   relative_label_run, relative_label_rise);
	  _update_buffer (&_plotter->outbuf);
	  _plotter->relative_label_run = relative_label_run;
	  _plotter->relative_label_rise = relative_label_rise;
	}
    }

  /* compute index of font in master table of PCL fonts, in g_fontdb.h */
  master_font_index =
    (_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* compute character sizes (width / height) for SR or SI instruction */
  /* also set need_new_size=true if needed */

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
  
  if (symbol_set != _plotter->pcl_symbol_set
      || spacing != _plotter->pcl_spacing
      || posture != _plotter->pcl_posture
      || stroke_weight != _plotter->pcl_stroke_weight
      || typeface != _plotter->pcl_typeface)
    need_new_font = need_new_size = true;
  
  if (need_new_font)
    {
      if (spacing == FIXED_SPACING)
	sprintf (_plotter->outbuf.current, 
		 "SD1,%d,2,%d,3,%.3f,5,%d,6,%d,7,%d;",
		 symbol_set, spacing, (double)NOMINAL_CHARS_PER_INCH, 
		 posture, stroke_weight, typeface);
      else			/* variable-width font */
	sprintf (_plotter->outbuf.current, 
		 "SD1,%d,2,%d,4,%.3f,5,%d,6,%d,7,%d;",
		 symbol_set, spacing, (double)NOMINAL_POINT_SIZE, 
		 posture, stroke_weight, typeface);
      _update_buffer (&_plotter->outbuf);

      _plotter->pcl_symbol_set = symbol_set;
      _plotter->pcl_spacing = spacing;
      _plotter->pcl_posture = posture;
      _plotter->pcl_stroke_weight = stroke_weight;
      _plotter->pcl_typeface = typeface;
    }

  /* compute character slant (device frame) */
  if (len == 0.0 || perplen == 0.0) /* a bad situation */
    tan_slant = 0.0;
  else
    {
      sin_slant = (dx * perpdx + dy * perpdy) / (len * perplen);
      cos_slant = sqrt (1 - sin_slant * sin_slant);
      tan_slant = sin_slant / cos_slant;
    }

  /* compute horizontal and vertical character sizes as percentages of
     the horizontal and vertical distances between scaling points P1 and
     P2, and specify them with the SR instruction (the 0.5 and 0.7 are HP
     magic...) */
  {
    int orientation = _plotter->drawstate->transform.nonreflection ? 1 : -1;
    double fractional_char_width, fractional_char_height;    
    double relative_char_width, relative_char_height;

    fractional_char_width = 0.5;
    fractional_char_height = 0.7;

    relative_char_width = fractional_char_width * 100 * len / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
    relative_char_height = 
      fractional_char_height * 100 * orientation * cos_slant * perplen / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
    
    if (need_new_size || 
	(relative_char_width != _plotter->relative_char_width
	 || relative_char_height != _plotter->relative_char_height))
      {
	sprintf (_plotter->outbuf.current, "SR%.3f,%.3f;", 
		 relative_char_width, relative_char_height);
	_update_buffer (&_plotter->outbuf);
	_plotter->relative_char_width = relative_char_width;
	_plotter->relative_char_height = relative_char_height;
      }
  }

  /* update slant angle if necessary */
  if (tan_slant != _plotter->char_slant_tangent)
    {
      sprintf (_plotter->outbuf.current, "SL%.3f;", tan_slant);
      _update_buffer (&_plotter->outbuf);
      _plotter->char_slant_tangent = tan_slant;
    }

  return;
}
