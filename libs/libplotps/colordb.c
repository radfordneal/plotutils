/* This file contains device-specific color computation routines.  These
   routines are called by functions in libplotps. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
int _idraw_pseudocolor P__((int red, int green, int blue));
void _compute_idraw_bgcolor P__((void));
#undef P__

/* we call this routine to evaluate _drawstate->ps_fgcolor lazily,
   i.e. only when needed (just before an object is written to the output
   buffer) */

void
_evaluate_ps_fgcolor()
{
  /* evaluate ps_fgcolor lazily, i.e. only when needed */
  if (_drawstate->ps_fgcolor_defined)
    return;

  _drawstate->ps_fgcolor_red = ((double)((_drawstate->fgcolor).red))/0xFFFF;
  _drawstate->ps_fgcolor_green = ((double)((_drawstate->fgcolor).green))/0xFFFF;
  _drawstate->ps_fgcolor_blue = ((double)((_drawstate->fgcolor).blue))/0xFFFF;

  /* quantize for idraw */
  _drawstate->idraw_fgcolor = _idraw_pseudocolor ((_drawstate->fgcolor).red,
						  (_drawstate->fgcolor).green,
						  (_drawstate->fgcolor).blue);
  _drawstate->ps_fgcolor_defined = TRUE;

  return;
}

/* we call this routine to evaluate _drawstate->ps_fillcolor lazily,
   i.e. only when needed (just before an object is written to the output
   buffer) */

void
_evaluate_ps_fillcolor()
{
  double red, green, blue;
  double desaturate;

  /* evaluate ps_fillcolor lazily, i.e. only when needed */
  if (_drawstate->ps_fillcolor_defined)
    return;

  if (_drawstate->fill_level == 0)
    {
      /* don't do anything, fill color will be ignored when writing objects*/
      _drawstate->ps_fillcolor_defined = TRUE;
      return;
    }

  red = ((double)((_drawstate->fillcolor).red))/0xFFFF;
  green = ((double)((_drawstate->fillcolor).green))/0xFFFF;
  blue = ((double)((_drawstate->fillcolor).blue))/0xFFFF;

  /* fill_level, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_drawstate->fill_level - 1.)/0xFFFE;

  _drawstate->ps_fillcolor_red = red + desaturate * (1.0 - red);
  _drawstate->ps_fillcolor_green = green + desaturate * (1.0 - green);
  _drawstate->ps_fillcolor_blue = blue + desaturate * (1.0 - blue);

  /* next subroutine needs fields that this will fill in... */
  _evaluate_ps_fgcolor();

  /* quantize for idraw, in a complicated way; we can choose from among a
     finite discrete set of values for idraw_bgcolor and idraw_shading, to
     approximate the fill color */
  _compute_idraw_bgcolor();
  
  _drawstate->ps_fillcolor_defined = TRUE;
  return;
}

/* find closest known point within the RGB color cube, using Euclidean
   distance as our metric */
int
_idraw_pseudocolor (red, green, blue)
     int red, green, blue;
{
  double difference;
  int i;
  int best = 0;
  
  difference = MAXDOUBLE;
  for (i = 0; i < IDRAW_NUM_STD_COLORS; i++)
    {
      double newdifference;
      
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
   shading that will most closely match the user-specified fill color.  It
   is called only when the elements ps_fillcolor_*, idraw_fgcolor_* of the
   drawing state have been filled in. */

void
_compute_idraw_bgcolor()
{
  double truered, truegreen, trueblue;
  double fgred, fggreen, fgblue;
  double difference = MAXDOUBLE;
  int i, j;
  int best_bgcolor = 0, best_shading = 0;

  truered = 0xFFFF * _drawstate->ps_fillcolor_red;
  truegreen = 0xFFFF * _drawstate->ps_fillcolor_green;
  trueblue = 0xFFFF * _drawstate->ps_fillcolor_blue;

  fgred = (double)(_idraw_stdcolors[_drawstate->idraw_fgcolor].red);
  fggreen = (double)(_idraw_stdcolors[_drawstate->idraw_fgcolor].green);
  fgblue = (double)(_idraw_stdcolors[_drawstate->idraw_fgcolor].blue);

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
	    }
	}
    }

  _drawstate->idraw_bgcolor = best_bgcolor;
  _drawstate->idraw_shading = best_shading;

}
