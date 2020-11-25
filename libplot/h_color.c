#include "sys-defines.h"
#include "extern.h"

#define ONEBYTE (0xff)

#define USE_PEN_ZERO (_plotter->hpgl_version == 2 && (_plotter->hpgl_use_opaque_mode || _plotter->hpgl_have_palette))

/* _h_set_pencolor() sets the physical pen color to match the pen color in
   our current drawing state.  If the palette contains a matching color,
   the corresponding pen is selected.  Otherwise, the color is entered into
   the palette and the corresponding pen is selected.  If the palette
   cannot be modified, i.e., new pen colors cannot be `soft-defined', the
   palette is searched for the closest match, and the corresponding
   `quantized' pen is selected.
   
   We adopt a convention: nonwhite pen colors are never quantized to white.

   Pen #0 is the canonical white pen.  But on pen plotters, drawing with
   pen #0 isn't meaningful.  So we don't actually use pen #0 to draw with
   unless HPGL_VERSION==2 and HPGL_OPAQUE_MODE=yes (or
   HPGL_ASSIGN_COLORS=yes, which presumably means the output is directed to
   a DesignJet).  Accordingly if the closest match here is pen #0, we set
   the advisory `bad_pen' flag in the Plotter object to `true'; otherwise
   we set it to `false'. */

void
#ifdef _HAVE_PROTOS
_h_set_pen_color(S___(Plotter *_plotter))
#else
_h_set_pen_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int longred, longgreen, longblue;
  int red, green, blue;
  int i;
  plColor color;
  
  color = _plotter->drawstate->fgcolor;
  longred = color.red;
  longgreen = color.green;
  longblue = color.blue;

  /* truncate to 24-bit color */
  red = (longred >> 8) & ONEBYTE;
  green = (longgreen >> 8) & ONEBYTE;
  blue = (longblue >> 8) & ONEBYTE;
  
  /* check whether color is already in the palette */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    {
      if (_plotter->pen_defined[i] != 0 /* i.e. defined (hard or soft) */
	  && _plotter->pen_color[i].red == red
	  && _plotter->pen_color[i].green == green
	  && _plotter->pen_color[i].blue == blue)
	/* color is in palette */
	{
	  if (i != 0 || (i == 0 && USE_PEN_ZERO))
	    /* can be selected */
	    {
	      _set_hpgl_pen (R___(_plotter) i);
	      _plotter->hpgl_bad_pen = false;
	    }
	  else
	    /* aren't using pen #0, so set advisory flag */
	    _plotter->hpgl_bad_pen = true;

	  return;
	}
    }      

  /* color not in palette, must do something */
  if (_plotter->hpgl_version == 2 && _plotter->hpgl_have_palette)
    /* can soft-define pen colors (HP-GL/2, presumably a DesignJet) */
    {
      /* assign current `free pen' to be the new color */
      sprintf (_plotter->page->point, "PC%d,%d,%d,%d;", 
	       _plotter->hpgl_free_pen, red, green, blue);
      _update_buffer (_plotter->page);
      _plotter->pen_color[_plotter->hpgl_free_pen].red = red;
      _plotter->pen_color[_plotter->hpgl_free_pen].green = green;
      _plotter->pen_color[_plotter->hpgl_free_pen].blue = blue;
      _plotter->pen_defined[_plotter->hpgl_free_pen] = 1; /* soft-defined */
      /* select pen */
      _set_hpgl_pen (R___(_plotter) _plotter->hpgl_free_pen);
      /* update free pen, i.e. choose next non-hard-defined pen */
      do
	_plotter->hpgl_free_pen = (_plotter->hpgl_free_pen + 1) % HPGL2_MAX_NUM_PENS;
      while (_plotter->pen_defined[_plotter->hpgl_free_pen] == 2);

      _plotter->hpgl_bad_pen = false;
      return;
    }
  else
    /* HPGL_VERSION may be "1" (i.e. generic HP-GL) or "1.5"
       (i.e. HP7550A), or "2" (i.e. modern HP-GL/2, but without the ability
       to define a palette).  So select closest defined pen in RGB cube,
       using Euclidean distance as metric.  Final arg here is `true' on
       account of our convention that a non-white pen color [unlike a fill
       color] is never quantized to white (i.e. to pen #0). */
    {
      i = _hpgl_pseudocolor (R___(_plotter) red, green, blue, true);
      if (i != 0 || (i == 0 && USE_PEN_ZERO))
	/* can be selected */
	{
	  _set_hpgl_pen (R___(_plotter) i);
	  _plotter->hpgl_bad_pen = false;
	}
      else
	/* aren't using pen #0, so set advisory flag */
	_plotter->hpgl_bad_pen = true;

      return;
    }
}

/* _h_set_fill_color() is similar to _h_set_pen_color: it sets the physical
   pen color and fill type to match the fill color in our current drawing
   state.  For HP7550A plotters and HP-GL/2 devices, that is; we don't
   support filling on HP-GL devices.  There are three possibilities.

   (1) For HP-GL/2 devices supporting modification of the palette,
   i.e. `soft-definition' of pen colors, we use solid filling, after
   defining the fill color as a new pen color if necessary.

   (2) For HP-GL/2 not supporting soft-definition of pen colors, we use
   shading.  So we must determine which shade of which defined pen is
   closest to the fill color in the sense of Euclidean distance within the
   RGB cube.  `Shades' are desaturations (interpolations between a pen
   color, and white).

   (3) For pre-HP-GL/2 devices, we use solid filling (shading is not
   supported), so we determine which defined pen is closest to the fill
   color in the sense of Euclidean distance within the RGB cube.

   Pen #0 is the canonical white pen.  But on pen plotters, filling with
   pen #0 isn't meaningful.  So we don't actually use pen #0 to fill with
   unless HPGL_VERSION==2 and HPGL_OPAQUE_MODE=yes (or
   HPGL_ASSIGN_COLORS=yes, which presumably means the output is directed to
   a DesignJet).  Accordingly if the closest match here is pen #0, we set
   the advisory `bad_pen' flag in the Plotter object to `true'; otherwise
   we set it to `false'. */

void
#ifdef _HAVE_PROTOS
_h_set_fill_color(S___(Plotter *_plotter))
#else
_h_set_fill_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double d_red, d_green, d_blue, desaturate;
  int longred, longgreen, longblue;
  int red, green, blue;
  int level, i;
  plColor color;
  
  level = _plotter->drawstate->fill_type;
  if (level == 0) /* won't be doing filling, so don't set pen */
    return;

  color = _plotter->drawstate->fillcolor;
  d_red = ((double)(color.red))/0xFFFF;
  d_green = ((double)(color.green))/0xFFFF;
  d_blue = ((double)(color.blue))/0xFFFF;

  /* fill_type, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)level - 1.)/0xFFFE;

  longred = IROUND(0xFFFF * (d_red + desaturate * (1.0 - d_red)));
  longgreen = IROUND(0xFFFF * (d_green + desaturate * (1.0 - d_green)));
  longblue = IROUND(0xFFFF * (d_blue + desaturate * (1.0 - d_blue)));

  /* truncate to 24-bit color */
  red = (longred >> 8) & ONEBYTE;
  green = (longgreen >> 8) & ONEBYTE;
  blue = (longblue >> 8) & ONEBYTE;
  
  /* check whether color is already in palette */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    {
      if (_plotter->pen_defined[i] != 0
	  && _plotter->pen_color[i].red == red
	  && _plotter->pen_color[i].green == green
	  && _plotter->pen_color[i].blue == blue)
	/* color is in palette */
	{
	  if (i != 0 || (i == 0 && USE_PEN_ZERO))
	    /* can be selected */
	    {
	      _set_hpgl_pen (R___(_plotter) i);
	      /* set fill type to solid, unidirectional */
	      _set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SOLID_UNI, 0.0);
	      _plotter->hpgl_bad_pen = false;
	    }
	  else
	    /* aren't using pen #0, so set advisory flag */
	    _plotter->hpgl_bad_pen = true;

	  return;
	}
    }      

  /* color not in palette, must do something */
  if (_plotter->hpgl_version == 2 && _plotter->hpgl_have_palette)
    /* can soft-define pen colors (HP-GL/2) */
    {
      /* assign current `free pen' to be the new color */
      sprintf (_plotter->page->point, "PC%d,%d,%d,%d;", 
	       _plotter->hpgl_free_pen, red, green, blue);
      _update_buffer (_plotter->page);
      _plotter->pen_color[_plotter->hpgl_free_pen].red = red;
      _plotter->pen_color[_plotter->hpgl_free_pen].green = green;
      _plotter->pen_color[_plotter->hpgl_free_pen].blue = blue;
      _plotter->pen_defined[_plotter->hpgl_free_pen] = 1; /* soft-defined */
      /* select pen */
      _set_hpgl_pen (R___(_plotter) _plotter->hpgl_free_pen);
      /* update free pen, i.e. choose next non-hard-defined pen */
      do
	_plotter->hpgl_free_pen = (_plotter->hpgl_free_pen + 1) % HPGL2_MAX_NUM_PENS;
      while (_plotter->pen_defined[_plotter->hpgl_free_pen] == 2);
      /* set fill type to solid, unidirectional */
      _set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SOLID_UNI, 0.0);

      _plotter->hpgl_bad_pen = false;
      return;
    }

  else if (_plotter->hpgl_version == 2 && _plotter->hpgl_have_palette == false)
    /* HP-GL/2, but can't soft-define pen colors; locate closest point
       in RGB cube that is a desaturated version of one of the defined
       pen colors, and fill by shading at the appropriate level */
    {
      double shading;

      _compute_pseudo_fillcolor (R___(_plotter) 
				 red, green, blue, &i, &shading);

      if (i != 0 || (i == 0 && USE_PEN_ZERO))
	/* can be selected */
	{
	  _set_hpgl_pen (R___(_plotter) i);
	  /* shading level in HP-GL/2 is expressed as a percentage */
	  _set_hpgl_fill_type (R___(_plotter) 
			     HPGL_FILL_SHADING, 100.0 * shading);
	  _plotter->hpgl_bad_pen = false;
	}
      else
	/* aren't using pen #0, so set advisory flag */
	_plotter->hpgl_bad_pen = true;
      
      return;
    }

  else
    /* HPGL_VERSION must be "1" (i.e. generic HP-GL) or "1.5"
       (i.e. HP7550A), so select closest defined pen in RGB cube, using
       Euclidean distance as metric.  Final arg is set to `false' to
       signify that nonwhite fill colors [unlike pen colors] may be
       quantized to white, i.e. to pen #0. */
    {
      int i;
      
      i = _hpgl_pseudocolor (R___(_plotter) red, green, blue, false);

      if (i != 0 || (i == 0 && USE_PEN_ZERO)) /* USE_PEN_ZERO = false here */
	/* can be selected */
	{
	  _set_hpgl_pen (R___(_plotter) i);
	  /* set fill type to solid, unidirectional */
	  _set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SOLID_UNI, 0.0);
	  _plotter->hpgl_bad_pen = false;
	}
      else
	/* aren't using pen #0, so set advisory flag */
	_plotter->hpgl_bad_pen = true;
      
      return;
    }
}

void 
#ifdef _HAVE_PROTOS
_set_hpgl_pen (R___(Plotter *_plotter) int new_pen)
#else
_set_hpgl_pen (R___(_plotter) new_pen)
     S___(Plotter *_plotter;)
     int new_pen;
#endif
{
  if (new_pen != _plotter->pen)	/* need to select new pen */
    {
      if (_plotter->hpgl_pendown)
	{
	  sprintf (_plotter->page->point, "PU;");
	  _update_buffer (_plotter->page);
	  _plotter->hpgl_pendown = false;
	}
      sprintf (_plotter->page->point, "SP%d;", new_pen);
      _update_buffer (_plotter->page);
      _plotter->pen = new_pen;
    }
  return;
}

void
#ifdef _HAVE_PROTOS
_set_hpgl_fill_type (R___(Plotter *_plotter) int new_hpgl_fill_type, double option1)
#else
_set_hpgl_fill_type (R___(_plotter) new_hpgl_fill_type, option1)
     S___(Plotter *_plotter;)
     int new_hpgl_fill_type;
     double option1;
#endif
{
  if (new_hpgl_fill_type != _plotter->hpgl_fill_type
      || (new_hpgl_fill_type == HPGL_FILL_SHADING 
	  && _plotter->hpgl_shading_level != option1))
    /* need to emit `FT' instruction */
    {
      switch (new_hpgl_fill_type)
	{
	case HPGL_FILL_SOLID_BI:
	case HPGL_FILL_SOLID_UNI:	  
	default:
	  /* options ignored */
	  sprintf (_plotter->page->point, "FT%d;", new_hpgl_fill_type);
	  break;
	case HPGL_FILL_SHADING:
	  /* option1 is shading level in percent */
	  sprintf (_plotter->page->point, "FT%d,%.1f;", 
		   new_hpgl_fill_type, option1);
	  _plotter->hpgl_shading_level = option1;
	  break;
	/* hatching and cross-hatching, anyone? */
	}
      _update_buffer (_plotter->page);
      _plotter->hpgl_fill_type = new_hpgl_fill_type;
    }
  return;
}

/* Find closest point within the RGB color cube that is a defined pen
   color, using Euclidean distance as our metric.  Final arg, if set,
   specifies that nonwhite colors should never be quantized to white. */
int
#ifdef _HAVE_PROTOS
_hpgl_pseudocolor (R___(Plotter *_plotter) int red, int green, int blue, bool restrict_white)
#else
_hpgl_pseudocolor (R___(_plotter) red, green, blue, restrict_white)
     S___(Plotter *_plotter;)
     int red, green, blue;
     bool restrict_white;
#endif
{
  unsigned long int difference = INT_MAX;
  int i;
  int best = 0;

  if (red == 0xff && green == 0xff && blue == 0xff)
    /* white pen */
    return 0;

  for (i = (restrict_white ? 1 : 0); i < HPGL2_MAX_NUM_PENS; i++)
    {
      if (_plotter->pen_defined[i] != 0)
	{
	  unsigned long int newdifference;
	  int ored, ogreen, oblue;
	  
	  ored = _plotter->pen_color[i].red;
	  ogreen = _plotter->pen_color[i].green;
	  oblue = _plotter->pen_color[i].blue;
	  newdifference = ((red - ored) * (red - ored)
			   + (green - ogreen) * (green - ogreen)
			   + (blue - oblue) * (blue - oblue));
	  
	  if (newdifference < difference)
	    {
	      difference = newdifference;
	      best = i;
	    }
	}
    }
  return best;
}

/* locate closest point in RGB cube that is a desaturated ("shaded")
   version of one of the defined pen colors, using Euclidean distance as
   our metric */
void
#ifdef _HAVE_PROTOS
_compute_pseudo_fillcolor (R___(Plotter *_plotter) int red, int green, int blue, int *pen_ptr, double *shading_ptr)
#else
_compute_pseudo_fillcolor (R___(_plotter) red, green, blue, pen_ptr, shading_ptr)
     S___(Plotter *_plotter;)
     int red, green, blue;
     int *pen_ptr;
     double *shading_ptr;
#endif
{
  int best = 0;
  int i;
  double best_shading = 0.0;
  double difference = INT_MAX;
  double red_shifted, green_shifted, blue_shifted;
  
  /* shift color vector so that it emanates from `white' */
  red_shifted = (double)(red - 0xff);
  green_shifted = (double)(green - 0xff);
  blue_shifted = (double)(blue - 0xff);

  /* begin with pen #1 */
  for (i = 1; i < HPGL2_MAX_NUM_PENS; i++)
    {
      int ored, ogreen, oblue;
      double ored_shifted, ogreen_shifted, oblue_shifted;
      double red_proj_shifted, green_proj_shifted, blue_proj_shifted;
      double reciprocal_normsquared, dotproduct;
      double newdifference, shading;
      
      /* skip undefined pens */
      if (_plotter->pen_defined[i] == 0)
	continue;
      
      /* shift each pen color vector so that it emanates from `white' */
      ored = _plotter->pen_color[i].red;
      ogreen = _plotter->pen_color[i].green;
      oblue = _plotter->pen_color[i].blue;
      /* if luser specified a white pen, skip it to avoid division by 0 */
      if (ored == 0xff && ogreen == 0xff && oblue == 0xff)
	continue;
      ored_shifted = (double)(ored - 0xff);
      ogreen_shifted = (double)(ogreen - 0xff);
      oblue_shifted = (double)(oblue - 0xff);

      /* project shifted color vector onto shifted pen color vector */
      reciprocal_normsquared = 1.0 / (ored_shifted * ored_shifted
				      + ogreen_shifted * ogreen_shifted
				      + oblue_shifted * oblue_shifted);
      dotproduct = (red_shifted * ored_shifted
		    + green_shifted * ogreen_shifted
		    + blue_shifted * oblue_shifted);
      shading = reciprocal_normsquared * dotproduct;
      
      red_proj_shifted = shading * ored_shifted;
      green_proj_shifted = shading * ogreen_shifted;
      blue_proj_shifted = shading * oblue_shifted;
      
      newdifference = (((red_proj_shifted - red_shifted) 
			* (red_proj_shifted - red_shifted))
		       + ((green_proj_shifted - green_shifted) 
			  * (green_proj_shifted - green_shifted))
		       + ((blue_proj_shifted - blue_shifted) 
			  * (blue_proj_shifted - blue_shifted)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	  best_shading = shading;
	}
    }

  /* compensate for roundoff error */
  if (best_shading <= 0.0)
    best_shading = 0.0;

  *pen_ptr = best;
  *shading_ptr = best_shading;
  return;
}
