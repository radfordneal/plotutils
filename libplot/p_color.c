/* This file contains device-specific color computation routines.  They are
   called by various PSPlotter methods, before drawing objects.  They set
   the appropriate PSPlotter-specific fields in the drawing state. */

#include "sys-defines.h"
#include "extern.h"

/* forward references */
static int _idraw_pseudocolor ____P((int red, int green, int blue));

/* we call this routine to evaluate _plotter->drawstate->ps_fgcolor lazily,
   i.e. only when needed (just before an object is written to the output
   buffer) */

void
#ifdef _HAVE_PROTOS
_p_set_pen_color(void)
#else
_p_set_pen_color()
#endif
{
  _plotter->drawstate->ps_fgcolor_red = 
    ((double)((_plotter->drawstate->fgcolor).red))/0xFFFF;
  _plotter->drawstate->ps_fgcolor_green = 
    ((double)((_plotter->drawstate->fgcolor).green))/0xFFFF;
  _plotter->drawstate->ps_fgcolor_blue = 
    ((double)((_plotter->drawstate->fgcolor).blue))/0xFFFF;

  /* quantize for idraw */
  _plotter->drawstate->ps_idraw_fgcolor = 
    _idraw_pseudocolor ((_plotter->drawstate->fgcolor).red,
			(_plotter->drawstate->fgcolor).green,
			(_plotter->drawstate->fgcolor).blue);

  return;
}

/* we call this routine to evaluate _plotter->drawstate->ps_fillcolor lazily,
   i.e. only when needed (just before an object is written to the output
   buffer) */

void
#ifdef _HAVE_PROTOS
_p_set_fill_color(void)
#else
_p_set_fill_color()
#endif
{
  double red, green, blue;
  double desaturate;

  if (_plotter->drawstate->fill_level == 0)
    /* don't do anything, fill color will be ignored when writing objects*/
    return;

  red = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
  green = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
  blue = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;

  /* fill_level, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_plotter->drawstate->fill_level - 1.)/0xFFFE;

  _plotter->drawstate->ps_fillcolor_red = red + desaturate * (1.0 - red);
  _plotter->drawstate->ps_fillcolor_green = green + desaturate * (1.0 - green);
  _plotter->drawstate->ps_fillcolor_blue = blue + desaturate * (1.0 - blue);

  /* next subroutine needs fields that this will fill in... */
  _plotter->set_pen_color();

  /* Quantize for idraw, in a complicated way; we can choose from among a
     finite discrete set of values for ps_idraw_bgcolor and
     ps_idraw_shading, to approximate the fill color.  We also adjust
     ps_fillcolor_* because the PS interpreter will use the
     ps_idraw_shading variable to interpolate between fgcolor and bgcolor,
     i.e. fgcolor and fillcolor. */
  _p_compute_idraw_bgcolor();
  
  return;
}

/* Find closest known point within the RGB color cube, using Euclidean
   distance as our metric.  Our convention: no non-white color should 
   be mapped to white. */
static int
#ifdef _HAVE_PROTOS
_idraw_pseudocolor (int red, int green, int blue)
#else
_idraw_pseudocolor (red, green, blue)
     int red, green, blue;
#endif
{
  double difference;
  int i;
  int best = 0;
  
  difference = DBL_MAX;
  for (i = 0; i < IDRAW_NUM_STD_COLORS; i++)
    {
      double newdifference;
      
      if (_idraw_stdcolors[i].red == 0xffff
	  && _idraw_stdcolors[i].green == 0xffff
	  && _idraw_stdcolors[i].blue == 0xffff)
	/* white is a possible quantization only for white itself (our
           convention) */
	{
	  if (red == 0xffff && green == 0xffff && blue == 0xffff)
	    {
	      difference = 0.0;
	      best = i;
	    }
	  continue;
	}

      newdifference = ((double)(_idraw_stdcolors[i].red - red)
		       * (double)(_idraw_stdcolors[i].red - red))
		    + ((double)(_idraw_stdcolors[i].green - green) 
		       * (double)(_idraw_stdcolors[i].green - green)) 
		    + ((double)(_idraw_stdcolors[i].blue - blue)
		       * (double)(_idraw_stdcolors[i].blue - blue));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }

  return best;
}

/* Once the idraw foreground color (i.e. quantized pen color) has been
   determined, this routine computes the idraw background color and idraw
   shading (0.0, 0.25, 0.5, 0.75, or 1.0) that will most closely match the
   user-specified fill color.  It is called only when the elements
   ps_fillcolor_*, ps_idraw_fgcolor_* of the drawing state have been filled in.

   At the end of this function we adjust ps_fillcolor_* so that the output
   file will produce similar colors when parsed both by idraw and the PS
   interpreter.  In fact we can persuade the PS interpreter to produce
   exactly the fill color specified by the user, except when the idraw
   shading is 0.0.  In that case the fill color must be the same as the pen
   color.  That situation will occur only if the user-specified fill color
   is very close to the user-specified pen color. */

void
#ifdef _HAVE_PROTOS
_p_compute_idraw_bgcolor(void)
#else
_p_compute_idraw_bgcolor()
#endif
{
  double truered, truegreen, trueblue;
  double fgred, fggreen, fgblue;
  double difference = DBL_MAX;
  int i, j;
  int best_bgcolor = 0, best_shading = 0;
  double best_shade = 0.0;

  truered = 0xFFFF * _plotter->drawstate->ps_fillcolor_red;
  truegreen = 0xFFFF * _plotter->drawstate->ps_fillcolor_green;
  trueblue = 0xFFFF * _plotter->drawstate->ps_fillcolor_blue;

  fgred = (double)(_idraw_stdcolors[_plotter->drawstate->ps_idraw_fgcolor].red);
  fggreen = (double)(_idraw_stdcolors[_plotter->drawstate->ps_idraw_fgcolor].green);
  fgblue = (double)(_idraw_stdcolors[_plotter->drawstate->ps_idraw_fgcolor].blue);

  for (i = 0; i < IDRAW_NUM_STD_COLORS; i++)
    {
      double bgred, bggreen, bgblue;

      bgred = (double)(_idraw_stdcolors[i].red);
      bggreen = (double)(_idraw_stdcolors[i].green);
      bgblue = (double)(_idraw_stdcolors[i].blue);

      for (j = 0; j < IDRAW_NUM_STD_SHADINGS; j++)
	{
	  double approxred, approxgreen, approxblue;
	  double shade, newdifference;
	  
	  shade = _idraw_stdshadings[j];
	  
	  approxred = shade * bgred + (1.0 - shade) * fgred;
	  approxgreen = shade * bggreen + (1.0 - shade) * fggreen;
	  approxblue = shade * bgblue + (1.0 - shade) * fgblue;	  

	  newdifference = (truered - approxred) * (truered - approxred)
	    + (truegreen - approxgreen) * (truegreen - approxgreen)
	      + (trueblue - approxblue) * (trueblue - approxblue);
	  
	  if (newdifference < difference)
	    {
	      difference = newdifference;
	      best_bgcolor = i;
	      best_shading = j;
	      best_shade = shade;
	    }
	}
    }

  _plotter->drawstate->ps_idraw_bgcolor = best_bgcolor;
  _plotter->drawstate->ps_idraw_shading = best_shading;

  /* now adjust ps_fillcolor_* fields so that interpolation between
     ps_fgcolor_* and ps_fillcolor_*, as specified by the shade, will yield
     the user-specified fill color.  According to the PS prologue, the PS
     interpreter will compute a fill color thus:

     true_FILLCOLOR = shade * PS_FILLCOLOR + (1-shade) * PS_FGCOLOR

     we can compute an adjusted fillcolor thus:

     PS_FILLCOLOR = (true_FILLCOLOR - (1-shade) * PS_FGCOLOR) / shade.

     This is possible unless shade=0.0, in which case both idraw and the PS
     interpreter will use the pen color as the fill color. */

  if (best_shade != 0.0)
    {
      _plotter->drawstate->ps_fillcolor_red 
	= (_plotter->drawstate->ps_fillcolor_red 
	   - (1.0 - best_shade) * _plotter->drawstate->ps_fgcolor_red) / best_shade;
      _plotter->drawstate->ps_fillcolor_green
	= (_plotter->drawstate->ps_fillcolor_green
	   - (1.0 - best_shade) * _plotter->drawstate->ps_fgcolor_green) / best_shade;
      _plotter->drawstate->ps_fillcolor_blue
	= (_plotter->drawstate->ps_fillcolor_blue
	   - (1.0 - best_shade) * _plotter->drawstate->ps_fgcolor_blue) / best_shade;
    }
}
