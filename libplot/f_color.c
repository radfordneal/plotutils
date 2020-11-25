/* This file contains device-specific color database access routines.  They
   are called by various FigPlotter methods, before drawing objects.  They
   set the appropriate FigPlotter-specific fields in the drawing state. */

#include "sys-defines.h"
#include "extern.h"

#define ONEBYTE 0xff

/* by setting this undocumented variable, user may request quantization of
   colors (no user-defined colors, only native xfig ones). */
#ifdef SUPPORT_FIG_COLOR_QUANTIZATION
int _libplotfig_use_pseudocolor = 0;
#endif

/* FIG_COLOR returns the index of the Fig color corresponding to specified
   a 48-bit RGB value.  This has (or may have) a side effect.  If the Fig
   color is not standard, it will be added to the database of user-defined
   colors, and a Fig `color pseudo-object' will be output later.

   We do not call this function whenever the user calls pencolor() or
   fillcolor(), since we don't want to fill up the database with colors
   that the user may not actually use.  Instead, we call it just before we
   write a colored object to the output buffer (lazy evaluation), by
   evaluating the f_set_pen_color() and f_set_fill_color() functions below.

   If the external variable use_pseudocolor is nonzero, we don't actually
   maintain a database of user-defined colors.  Instead we just quantize to
   one of xfig's native 32 colors.  (They provide a [rather strange]
   partition of the color cube; see f_color2.c.) */

/* forward references */
#ifdef SUPPORT_FIG_COLOR_QUANTIZATION
static int _fig_pseudocolor ____P((int red, int green, int blue));
#endif

int
#ifdef _HAVE_PROTOS
_fig_color(R___(Plotter *_plotter) int red, int green, int blue)
#else
_fig_color(R___(_plotter) red, green, blue)
     S___(Plotter *_plotter;)
     int red, green, blue;
#endif
{
  int fig_fgcolor_red, fig_fgcolor_green, fig_fgcolor_blue;
  long int fig_fgcolor_rgb;
  int i;

  /* xfig supports only 24-bit color */
  fig_fgcolor_red = (red >> 8) & ONEBYTE;
  fig_fgcolor_green = (green >> 8) & ONEBYTE;
  fig_fgcolor_blue = (blue >> 8) & ONEBYTE;

#ifdef SUPPORT_FIG_COLOR_QUANTIZATION
  if (_libplotfig_use_pseudocolor) /* quantize */
    return _fig_pseudocolor (fig_fgcolor_red, fig_fgcolor_green,
			    fig_fgcolor_blue);
#endif

  for (i=0; i<FIG_NUM_STD_COLORS; i++) /* search list of standard colors */
    {
      if ((_fig_stdcolors[i].red == fig_fgcolor_red)
	  && (_fig_stdcolors[i].green == fig_fgcolor_green)
	  && (_fig_stdcolors[i].blue == fig_fgcolor_blue))
	return FIG_STD_COLOR_MIN + i;
    }

  /* This is the 24-bit (i.e. 3-byte) integer used internally by xfig.
     We assume our long ints are wide enough to handle 3 bytes. */
  fig_fgcolor_rgb = (fig_fgcolor_red << 16) + (fig_fgcolor_green << 8)
		    + (fig_fgcolor_blue);
    
  for (i=0; i<_plotter->fig_num_usercolors; i++) /* search list of user-defined colors */
    {
      if (_plotter->fig_usercolors[i] == fig_fgcolor_rgb)
	return FIG_USER_COLOR_MIN + i;
    }

  /* wasn't found, need to define it */

  if (_plotter->fig_num_usercolors == FIG_MAX_NUM_USER_COLORS)
    {
      _plotter->warning (R___(_plotter) "supply of user-defined colors is exhausted");
      return -1;
    }
  
  /* create new user-defined color, will emit it in f_closepl.c */
  _plotter->fig_usercolors[_plotter->fig_num_usercolors] = fig_fgcolor_rgb;
  _plotter->fig_num_usercolors++;
  return FIG_USER_COLOR_MIN + _plotter->fig_num_usercolors - 1;
}

/* find closest known point within the RGB color cube, using Euclidean
   distance as our metric */
#ifdef SUPPORT_FIG_COLOR_QUANTIZATION
static int
#ifdef _HAVE_PROTOS
_fig_pseudocolor (int red, int green, int blue)
#else
_fig_pseudocolor (red, green, blue)
     int red, green, blue;
#endif
{
  unsigned long int difference = INT_MAX;
  int i;
  int best = 0;
  
  /* xfig supports only 24-bit color */
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < FIG_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      if (_fig_stdcolors[i].red == 0xff
	  && _fig_stdcolors[i].green == 0xff
	  && _fig_stdcolors[i].blue == 0xff)
	/* white is a possible quantization only for white itself (our
           convention) */
	{
	  if (red == 0xff && green == 0xff && blue == 0xff)
	    {
	      difference = 0;
	      best = i;
	    }
	  continue;
	}

      newdifference = (((_fig_stdcolors[i].red - red) 
			* (_fig_stdcolors[i].red - red))
		       + ((_fig_stdcolors[i].green - green) 
			  * (_fig_stdcolors[i].green - green))
		       + ((_fig_stdcolors[i].blue - blue) 
			  * (_fig_stdcolors[i].blue - blue)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return FIG_STD_COLOR_MIN + best;
}
#endif /* SUPPORT_FIG_COLOR_QUANTIZATION */

/* we call this routine to evaluate _plotter->drawstate->fig_fgcolor
   lazily, i.e. only when needed (just before an object is written to the
   output buffer) */
void
#ifdef _HAVE_PROTOS
_f_set_pen_color(S___(Plotter *_plotter))
#else
_f_set_pen_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* OOB switches to default color */
  if (((_plotter->drawstate->fgcolor).red > 0xffff) 
      || ((_plotter->drawstate->fgcolor).green > 0xffff) 
      || ((_plotter->drawstate->fgcolor).blue > 0xffff))
    _plotter->drawstate->fig_fgcolor = _default_drawstate.fig_fgcolor;
  else
    _plotter->drawstate->fig_fgcolor = 
      _fig_color (R___(_plotter) 
		  (_plotter->drawstate->fgcolor).red,
		  (_plotter->drawstate->fgcolor).green, 
		  (_plotter->drawstate->fgcolor).blue);
  return;
}

/* we call this routine to evaluate _plotter->drawstate->fig_fillcolor and
   _plotter->drawstate->fig_fill_level lazily, i.e. only when needed (just
   before an object is written to the output buffer) */

/* Note that fill_type, if nonzero, specifies the extent to which the
   nominal fill color should be desaturated.  1 means no desaturation,
   0xffff means complete desaturation (white). */
void
#ifdef _HAVE_PROTOS
_f_set_fill_color(S___(Plotter *_plotter))
#else
_f_set_fill_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double fill_level;

  /* OOB switches to default color */
  if (((_plotter->drawstate->fillcolor).red > 0xffff) 
      || ((_plotter->drawstate->fillcolor).green > 0xffff) 
      || ((_plotter->drawstate->fillcolor).blue > 0xffff))
    _plotter->drawstate->fig_fillcolor = _default_drawstate.fig_fillcolor;
  else
    _plotter->drawstate->fig_fillcolor = 
      _fig_color (R___(_plotter)
		  (_plotter->drawstate->fillcolor).red,
		  (_plotter->drawstate->fillcolor).green, 
		  (_plotter->drawstate->fillcolor).blue);
  
  /* Now that we know _drawstate->fig_fillcolor, we can compute the fig
     fill level that will match the user's requested fill level.  Fig fill
     level is interpreted in a color dependent way, as follows.  The value
     -1 is special; means no fill at all (objects will be transparent).
     For other values, this is the interpretation:
     
     Color = black or default:
     		fill = 0  -> white
		fill = 1  -> very light grey
		     .
		     .
	        fill = 19 -> very dark grey
     		fill = 20 -> black

     Color = all colors other than black or default, including white
   		   fill = 0  -> black
   		   fill = 1  -> color, very faint intensity
			.
			.
		   fill = 19 -> color, very bright intensity
		   fill = 20 -> color, full intensity

	So 1->20 give increasingly intense "shades" of the
	color, with 20 giving the color itself.  Values 20->40
	are increasingly desaturated "tints" of the color,
	ranging from the color itself (20) to white (40).  A
	tint is defined as the color mixed with white.  (Values
	21->40 are not used when the color is black or default,
	or white itself.) */

  fill_level = ((double)_plotter->drawstate->fill_type - 1.)/0xFFFE;

  /* OOB sets fill level to a non-OOB default value */
  if (fill_level > 1.)
    fill_level = ((double)_default_drawstate.fill_type - 1.)/0xFFFE;

  /* level = 0 turns off filling (objects will be transparent) */
  else if (fill_level < 0.)
    fill_level = -1.0;

  if (fill_level == -1.0)
    _plotter->drawstate->fig_fill_level = -1;
  else
    {
      switch (_plotter->drawstate->fig_fillcolor)
	{
	case C_WHITE:		/* can't desaturate white */
	  _plotter->drawstate->fig_fill_level = 20;
	  break;
	case C_BLACK:
	  _plotter->drawstate->fig_fill_level = IROUND(20.0 - 20.0 * fill_level);
	  break;
	default:		/* interpret fill level as a saturation */
	  _plotter->drawstate->fig_fill_level = IROUND(20.0 + 20.0 * fill_level);
	  break;
	}
    }
  return;
}
