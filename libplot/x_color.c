/* This file contains device-specific color computation routines.  These
   routines are called by various XPlotter methods. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* we call this routine to evaluate _plotter->drawstate->x_fgcolor lazily,
   i.e. only when needed (just before an object is written out). */

void
#ifdef _HAVE_PROTOS
_x_set_pen_color(void)
#else
_x_set_pen_color()
#endif
{
  XColor rgb;

  rgb.red = _plotter->drawstate->fgcolor.red;
  rgb.green = _plotter->drawstate->fgcolor.green;
  rgb.blue = _plotter->drawstate->fgcolor.blue;

  /* allocate color cell */
  if (XAllocColor (_plotter->dpy, _plotter->cmap, &rgb) == 0)
    {
      _plotter->warning ("color request failed, couldn't allocate color cell");
      return;
    }

  /* select pen color as foreground color in X GC */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc, rgb.pixel);

  /* save the display device's internal representation of foreground color */
  _plotter->drawstate->x_fgcolor = rgb.pixel;

  return;
}

/* we call this routine to evaluate _plotter->drawstate->x_fillcolor lazily,
   i.e. only when needed (just before an object is written out). */

void
#ifdef _HAVE_PROTOS
_x_set_fill_color(void)
#else
_x_set_fill_color()
#endif
{
  double red, green, blue;
  double desaturate;
  XColor rgb;

  if (_plotter->drawstate->fill_level == 0) /* transparent */
    /* don't do anything, fill color will be ignored when writing objects*/
    return;

  red = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
  green = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
  blue = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;

  /* fill_level, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_plotter->drawstate->fill_level - 1.)/0xFFFE;

  rgb.red = (short)IROUND((red + desaturate * (1.0 - red))*(0xFFFF));
  rgb.green = (short)IROUND((green + desaturate * (1.0 - green))*(0xFFFF));
  rgb.blue = (short)IROUND((blue + desaturate * (1.0 - blue))*(0xFFFF));

  /* allocate color cell */
  if (XAllocColor (_plotter->dpy, _plotter->cmap, &rgb) == 0)
    {
      _plotter->warning ("color request failed, couldn't allocate color cell");
      return;
    }
      
  /* select fill color as foreground color in X GC */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc, rgb.pixel);

  /* save the display device's internal representation of fill color */
  _plotter->drawstate->x_fillcolor = rgb.pixel;

  return;
}

