/* This file contains device-specific color database access routines.
   These routines are called by functions in libplotfig. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define ONEBYTE 0xff

/* by setting this undocumented variable, user may request quantization of
   colors (no user-defined colors, only native xfig ones). */
int _libplotfig_use_pseudocolor = 0;

long int _usercolors[FIG_MAX_NUM_USER_COLORS]; /* linear list, could be bettered */
int _num_usercolors = 0;		/* initial value */

/* FIG_COLOR returns the index of the Fig color corresponding to specified
   a 48-bit RGB value.  This has (or may have) a side effect.  If the Fig
   color is not standard, it will be added to the database of user-defined
   colors, and a Fig `color pseudo-object' will be output later.

   We do not call this function whenever the user calls color() or
   fillcolor(), since we don't want to fill up the database with colors
   that the user may not actually use.  Instead, we call it just before we
   write a colored object to the output buffer (lazy evaluation), by
   evaluating the evaluate_fig_fgcolor() and evaluate_fig_fillcolor()
   functions below.

   If the external variable use_pseudocolor is nonzero, we don't actually
   maintain a database of user-defined colors.  Instead we just quantize to
   one of xfig's native 32 colors.  (They provide a [rather strange]
   partition of the color cube; see colordb.h.) */

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static int _fig_pseudocolor P__((int red, int green, int blue));
#undef P__

int
_fig_color(red, green, blue)
  int red, green, blue;
{
  int fig_fgcolor_red, fig_fgcolor_green, fig_fgcolor_blue;
  long int fig_fgcolor_rgb;
  int i;

  /* xfig supports only 24-bit color */
  fig_fgcolor_red = (red >> 8) & ONEBYTE;
  fig_fgcolor_green = (green >> 8) & ONEBYTE;
  fig_fgcolor_blue = (blue >> 8) & ONEBYTE;

  if (_libplotfig_use_pseudocolor) /* quantize */
    return _fig_pseudocolor (fig_fgcolor_red, fig_fgcolor_green,
			    fig_fgcolor_blue);

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
    
  for (i=0; i<_num_usercolors; i++)  /* search list of user-defined colors */
    {
      if (_usercolors[i] == fig_fgcolor_rgb)
	return FIG_USER_COLOR_MIN + i;
    }

  /* wasn't found, need to define it */

  if (_num_usercolors == FIG_MAX_NUM_USER_COLORS)
    {
      fprintf(stderr, "libplot: out of user-defined colors, color() ignored.\n");
      return -1;
    }
  
  /* create new user-defined color */
  _usercolors[_num_usercolors] = fig_fgcolor_rgb;	/* store rgb for closepl() */
  _num_usercolors++;
  return FIG_USER_COLOR_MIN + _num_usercolors - 1;
}

/* find closest known point within the RGB color cube, using Euclidean
   distance as our metric */
static int
_fig_pseudocolor (red, green, blue)
     int red, green, blue;
{
  unsigned long int difference = MAXINT;
  int i;
  int best = 0;
  
  /* xfig supports only 24-bit color */
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < FIG_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      newdifference = (_fig_stdcolors[i].red - red) * (_fig_stdcolors[i].red - red)
	+ (_fig_stdcolors[i].green - green) * (_fig_stdcolors[i].green - green) 
	  + (_fig_stdcolors[i].blue - blue) * (_fig_stdcolors[i].blue - blue);
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return FIG_STD_COLOR_MIN + best;
}

/* we call this routine to evaluate _drawstate->fig_fgcolor lazily,
   i.e. only when needed (just before an object is written to the output
   buffer) */

void
_evaluate_fig_fgcolor()
{
  /* evaluate fig_fgcolor lazily, i.e. only when needed */
  if (_drawstate->fig_fgcolor_defined)
    return;

  /* OOB switches to default color */
  if (((_drawstate->fgcolor).red > 0xffff) 
      || ((_drawstate->fgcolor).green > 0xffff) 
      || ((_drawstate->fgcolor).blue > 0xffff))
    _drawstate->fig_fgcolor = _default_drawstate.fig_fgcolor;
  else
    _drawstate->fig_fgcolor = 
      _fig_color ((_drawstate->fgcolor).red,
		 (_drawstate->fgcolor).green, 
		 (_drawstate->fgcolor).blue);
  _drawstate->fig_fgcolor_defined = TRUE;
  return;
}

/* we call this routine to evaluate _drawstate->fig_fillcolor and
   _drawstate->fig_fill_level lazily, i.e. only when needed (just before an
   object is written to the output buffer) */

/* Note that fill_level, if nonzero, specifies the extent to which the
   nominal fill color should be desaturated.  1 means no desaturation,
   0xffff means complete desaturation (white). */
void
_evaluate_fig_fillcolor()
{
  double fill_level;

  if (_drawstate->fig_fillcolor_defined)
    return;

  /* OOB switches to default color */
  if (((_drawstate->fillcolor).red > 0xffff) 
      || ((_drawstate->fillcolor).green > 0xffff) 
      || ((_drawstate->fillcolor).blue > 0xffff))
    _drawstate->fig_fillcolor = _default_drawstate.fig_fillcolor;
  else
    _drawstate->fig_fillcolor = 
      _fig_color ((_drawstate->fillcolor).red,
		 (_drawstate->fillcolor).green, 
		 (_drawstate->fillcolor).blue);
  _drawstate->fig_fillcolor_defined = TRUE;
  
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

  fill_level = ((double)_drawstate->fill_level - 1.)/0xFFFE;

  /* OOB sets fill level to a non-OOB default value */
  if (fill_level > 1.)
    fill_level = ((double)_default_drawstate.fill_level - 1.)/0xFFFE;

  /* level = 0 turns off fill (objects will be transparent) */
  else if (fill_level < 0.)
    fill_level = -1.0;

  if (fill_level == -1.0)
    _drawstate->fig_fill_level = -1;
  else
    {
      switch (_drawstate->fig_fillcolor)
	{
	case C_WHITE:		/* can't desaturate white */
	  _drawstate->fig_fill_level = 20;
	  break;
	case C_BLACK:
	  _drawstate->fig_fill_level = IROUND(20.0 - 20.0 * fill_level);
	  break;
	default:		/* interpret fill level as a saturation */
	  _drawstate->fig_fill_level = IROUND(20.0 + 20.0 * fill_level);
	  break;
	}
    }
  return;
}
