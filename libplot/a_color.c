/* We call these routines, which set the Illustrator pen and fill colors,
   lazily i.e. only when needed (just before an object is written to the
   output buffer). */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_a_set_pen_color(S___(Plotter *_plotter))
#else
_a_set_pen_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double red, green, blue;
  double cyan, magenta, yellow, black, temp;

  /* convert from RGB to CMYK */
  red = ((double)((_plotter->drawstate->fgcolor).red))/0xFFFF;
  green = ((double)((_plotter->drawstate->fgcolor).green))/0xFFFF;
  blue = ((double)((_plotter->drawstate->fgcolor).blue))/0xFFFF;
  cyan = 1.0 - red;
  magenta = 1.0 - green;
  yellow = 1.0 - blue;
  temp = magenta < yellow ? magenta : yellow;
  black = cyan < temp ? cyan : temp;
  cyan -= black;
  magenta -= black;
  yellow -= black;

  if ((_plotter->ai_pen_cyan != cyan)
      || (_plotter->ai_pen_magenta != magenta)
      || (_plotter->ai_pen_yellow != yellow)
      || (_plotter->ai_pen_black != black))
    /* need to change pen CMYK */
    {
      sprintf (_plotter->page->point, "%.4f %.4f %.4f %.4f K\n",
	       cyan, magenta, yellow, black);
      _update_buffer (_plotter->page);
      _plotter->ai_pen_cyan = cyan;
      _plotter->ai_pen_magenta = magenta;  
      _plotter->ai_pen_yellow = yellow;
      _plotter->ai_pen_black = black;
    }
  
  /* keep track of which colors AI uses */
  if (_plotter->ai_pen_cyan > 0.0)
    _plotter->ai_cyan_used = true;
  if (_plotter->ai_pen_magenta > 0.0)
    _plotter->ai_magenta_used = true;
  if (_plotter->ai_pen_yellow > 0.0)
    _plotter->ai_yellow_used = true;
  if (_plotter->ai_pen_black > 0.0)
    _plotter->ai_black_used = true;
  
  return;
}

/* fill_type, if nonzero, specifies the extent to which the nominal fill
   color should be desaturated.  1 means no desaturation, 0xffff means
   complete desaturation (white).

   This may be invoked if fill_type is 0.  If so, it's a sign that the
   nominal fill color has temporarily been set to the current pen color
   (for a subtle reason; see comment in a_endpath.c).  So in that case, we
   treat 0 as 1. */
void
#ifdef _HAVE_PROTOS
_a_set_fill_color(S___(Plotter *_plotter))
#else
_a_set_fill_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int fill_type;
  double red, green, blue;
  double cyan, magenta, yellow, black, temp;
  double desaturate;

  red = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
  green = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
  blue = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;

  fill_type = _plotter->drawstate->fill_type;
  if (fill_type == 0)
    fill_type = 1;		/* see comment above */
  desaturate = ((double)fill_type - 1.)/0xFFFE;
  red = red + desaturate * (1.0 - red);
  green = green + desaturate * (1.0 - green);
  blue = blue + desaturate * (1.0 - blue);

  /* convert from RGB to CMYK */
  cyan = 1.0 - red;
  magenta = 1.0 - green;
  yellow = 1.0 - blue;
  temp = magenta < yellow ? magenta : yellow;
  black = cyan < temp ? cyan : temp;
  cyan -= black;
  magenta -= black;
  yellow -= black;

  if ((_plotter->ai_fill_cyan != cyan)
      || (_plotter->ai_fill_magenta != magenta)
      || (_plotter->ai_fill_yellow != yellow)
      || (_plotter->ai_fill_black != black))
    /* need to change AI fill CMYK */
    {
      sprintf (_plotter->page->point, "%.4f %.4f %.4f %.4f k\n",
	       cyan, magenta, yellow, black);
      _update_buffer (_plotter->page);
      _plotter->ai_fill_cyan = cyan;
      _plotter->ai_fill_magenta = magenta;  
      _plotter->ai_fill_yellow = yellow;
      _plotter->ai_fill_black = black;
    }
  
  /* keep track of which colors AI uses */
  if (_plotter->ai_fill_cyan > 0.0)
    _plotter->ai_cyan_used = true;
  if (_plotter->ai_fill_magenta > 0.0)
    _plotter->ai_magenta_used = true;
  if (_plotter->ai_fill_yellow > 0.0)
    _plotter->ai_yellow_used = true;
  if (_plotter->ai_fill_black > 0.0)
    _plotter->ai_black_used = true;
  
  return;
}
