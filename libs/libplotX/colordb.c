/* This file contains device-specific color computation routines.  These
   routines are called by functions in libplotX. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* we call this routine to evaluate _drawstate->x_fgcolor lazily,
   i.e. only when needed (just before an object is written out). */

void
_evaluate_x_fgcolor()
{
  XColor rgb;

  /* evaluate x_fgcolor lazily, i.e. only when needed */
  if (_drawstate->x_fgcolor_defined)
    return;

  rgb.red = (_drawstate->fgcolor).red;
  rgb.green = (_drawstate->fgcolor).green;
  rgb.blue = (_drawstate->fgcolor).blue;

  /* allocate color cell */
  if (XAllocColor (_xdata.dpy, _xdata.cmap, &rgb) == 0)
    fprintf (stderr, "libplot: couldn't allocate color cell, color unchanged\n");

  /* save the display device's internal representation of foreground color */
  _drawstate->x_fgcolor = rgb.pixel;
  _drawstate->x_fgcolor_defined = TRUE;

  return;
}

/* we call this routine to evaluate _drawstate->x_fillcolor lazily,
   i.e. only when needed (just before an object is written out). */

void
_evaluate_x_fillcolor()
{
  double red, green, blue;
  double desaturate;
  XColor rgb;

  /* evaluate x_fgcolor lazily, i.e. only when needed */

  /* evaluate x_fillcolor lazily, i.e. only when needed */
  if (_drawstate->x_fillcolor_defined)
    return;

  if (_drawstate->fill_level == 0) /* transparent */
    {
      /* don't do anything, fill color will be ignored when writing objects*/
      _drawstate->x_fillcolor_defined = TRUE;
      return;
    }

  red = ((double)((_drawstate->fillcolor).red))/0xFFFF;
  green = ((double)((_drawstate->fillcolor).green))/0xFFFF;
  blue = ((double)((_drawstate->fillcolor).blue))/0xFFFF;

  /* fill_level, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_drawstate->fill_level - 1.)/0xFFFE;

  rgb.red = (short)IROUND((red + desaturate * (1.0 - red))*(0xFFFF));
  rgb.green = (short)IROUND((green + desaturate * (1.0 - green))*(0xFFFF));
  rgb.blue = (short)IROUND((blue + desaturate * (1.0 - blue))*(0xFFFF));

  /* allocate color cell */
  if (XAllocColor (_xdata.dpy, _xdata.cmap, &rgb) == 0)
    fprintf (stderr, "libplot: couldn't allocate color cell, color unchanged\n");

  /* save the display device's internal representation of fill color */
  _drawstate->x_fillcolor = rgb.pixel;
  _drawstate->x_fillcolor_defined = TRUE;

  return;
}

