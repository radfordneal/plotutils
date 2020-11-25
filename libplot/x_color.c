/* This file contains device-specific color computation routines.  These
   routines are called by various XPlotter methods. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* forward references */
static bool _x_retrieve_color __P ((XColor *rgb_ptr));

/* we call this routine to set the foreground color in the X GC used for
   drawing, only when needed (just before an object is written out) */

void
#ifdef _HAVE_PROTOS
_x_set_pen_color(void)
#else
_x_set_pen_color()
#endif
{
  Color old, new1;
  XColor rgb;

  new1 = _plotter->drawstate->fgcolor;
  old = _plotter->drawstate->current_fgcolor; /* i.e. as stored in gc */
  if (new1.red == old.red && new1.green == old.green && new1.blue == old.blue
      && _plotter->drawstate->x_fgcolor_status)
    /* can use current color cell */
    return;

  rgb.red = new1.red;
  rgb.green = new1.green;
  rgb.blue = new1.blue;

  /* retrieve matching color cell, if possible */
  if (_x_retrieve_color (&rgb) == false)
    return;

  /* select pen color as foreground color in GC used for drawing */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc_fg, rgb.pixel);

  /* save the new pixel value */
  _plotter->drawstate->x_fgcolor = rgb.pixel;

  /* flag this as a genuine pixel value */
  _plotter->drawstate->x_fgcolor_status = true;

  /* update non-opaque representation of stored foreground color */
  _plotter->drawstate->current_fgcolor = new1;

  return;
}

/* we call this routine to set the foreground color in the X GC used for
   filling, only when needed (just before an object is written out) */

void
#ifdef _HAVE_PROTOS
_x_set_fill_color(void)
#else
_x_set_fill_color()
#endif
{
  Color old, new1;
  XColor rgb;
  double desaturate;
  double red, green, blue;
  int old_level, new_level;

  new_level = _plotter->drawstate->fill_level;
  if (new_level == 0) /* transparent */
    /* don't do anything, fill color will be ignored when writing objects*/
    return;

  old_level = _plotter->drawstate->current_fill_level; /* as used in GC */
  new1 = _plotter->drawstate->fillcolor;
  old = _plotter->drawstate->current_fillcolor; /* as used in GC */
  if (new1.red == old.red && new1.green == old.green && new1.blue == old.blue
      && new_level == old_level 
      && _plotter->drawstate->x_fillcolor_status)
    /* can use current color cell */
    return;

  red = ((double)(_plotter->drawstate->fillcolor.red))/0xFFFF;
  green = ((double)(_plotter->drawstate->fillcolor.green))/0xFFFF;
  blue = ((double)(_plotter->drawstate->fillcolor.blue))/0xFFFF;

  /* fill_level, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_plotter->drawstate->fill_level - 1.)/0xFFFE;

  rgb.red = (short)IROUND((red + desaturate * (1.0 - red))*(0xFFFF));
  rgb.green = (short)IROUND((green + desaturate * (1.0 - green))*(0xFFFF));
  rgb.blue = (short)IROUND((blue + desaturate * (1.0 - blue))*(0xFFFF));

  /* retrieve matching color cell, if possible */
  if (_x_retrieve_color (&rgb) == false)
    return;

  /* select pen color as foreground color in GC used for filling */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc_fill, rgb.pixel);

  /* save the new pixel value */
  _plotter->drawstate->x_fillcolor = rgb.pixel;

  /* flag this as a genuine pixel value */
  _plotter->drawstate->x_fillcolor_status = true;

  /* update non-opaque representation of stored fill color */
  _plotter->drawstate->current_fillcolor = new1;
  _plotter->drawstate->current_fill_level = new_level;

  return;
}

/* we call this routine to set the foreground color in the X GC used for
   erasing, only when needed (just before an erasure takes place) */

void
#ifdef _HAVE_PROTOS
_x_set_bg_color(void)
#else
_x_set_bg_color()
#endif
{
  Color old, new1;
  XColor rgb;

  new1 = _plotter->drawstate->bgcolor;
  old = _plotter->drawstate->current_bgcolor; /* i.e. as stored in gc */
  if (new1.red == old.red && new1.green == old.green && new1.blue == old.blue
      && _plotter->drawstate->x_bgcolor_status)
    /* can use current color cell */
    return;

  rgb.red = new1.red;
  rgb.green = new1.green;
  rgb.blue = new1.blue;

  /* retrieve matching color cell, if possible */
  if (_x_retrieve_color (&rgb) == false)
    return;

  /* select background color as foreground color in GC used for erasing */
  XSetForeground (_plotter->dpy, _plotter->drawstate->gc_bg, rgb.pixel);

  /* save the new pixel value */
  _plotter->drawstate->x_bgcolor = rgb.pixel;

  /* flag this as a genuine pixel value */
  _plotter->drawstate->x_bgcolor_status = true;

  /* update non-opaque representation of stored background color */
  _plotter->drawstate->current_bgcolor = new1;

  return;
}

/* This is the internal X color retrieval routine.  It first searches for a
   specified RGB in a cache of previously retrieved color cells, and if
   that fails, allocates a new color cell.  Return value indicates success.
   Cache is maintained as a linked list (not optimal, but it facilitates
   color cell management; see comment in x_erase.c). */

static bool 
#ifdef _HAVE_PROTOS
_x_retrieve_color (XColor *rgb_ptr)
#else
_x_retrieve_color (rgb_ptr)
     XColor *rgb_ptr;
#endif
{
  Colorrecord *cptr;
  int rgb_red = rgb_ptr->red;
  int rgb_green = rgb_ptr->green;  
  int rgb_blue = rgb_ptr->blue;
  int xretval;
  static bool color_warning_printed = false;

  /* search cache list */
  for (cptr = _plotter->x_colorlist; cptr; cptr = cptr->next)
    {
      XColor cached_rgb;

      cached_rgb = cptr->rgb;
      if (cached_rgb.red == rgb_red
	  && cached_rgb.green == rgb_green
	  && cached_rgb.blue == rgb_blue)
	{
	  cptr->frame = _plotter->frame_number; /* keep track of frame number */
	  if (cptr->allocated)
	    /* found in cache, copy stored pixel value */
	    {
	      *rgb_ptr = cached_rgb;
	      return true;
	    }
	  else
	    /* found in cache, but no pixel value was successfully
               retrieved */
	    return false;
	}
    }

  /* not in cache, so try to allocate a new color cell */
  xretval = XAllocColor (_plotter->dpy, _plotter->cmap, rgb_ptr);
  if (xretval == 0 && _plotter->type == PL_X11 
      && _plotter->private_cmap == false)
    /* failure, but if this is an X Plotter rather than an X Drawable
       Plotter, switch to private colormap and try again */
    {
      Colormap new_cmap;

      _plotter->warning ("color supply low, switching to private colormap");
      new_cmap = XCopyColormapAndFree (_plotter->dpy, _plotter->cmap);
      _plotter->private_cmap = true; /* at least, we tried */
      if (new_cmap == 0)
	{
	  _plotter->warning ("unable to create private colormap");
	  _plotter->warning ("color supply exhausted, can't create new colors");
	  color_warning_printed = true;

	  /* add null color cell to head of cache list */
	  cptr = (Colorrecord *)_plot_xmalloc (sizeof (Colorrecord));
	  cptr->rgb.red = rgb_red;
	  cptr->rgb.green = rgb_green;
	  cptr->rgb.blue = rgb_blue;
	  cptr->allocated = false;
	  cptr->frame = _plotter->frame_number; /* keep track of frame number */
	  cptr->next = _plotter->x_colorlist;
	  _plotter->x_colorlist = cptr;
	  return false;
	}
      _plotter->cmap = new_cmap;
      XSetWindowColormap (_plotter->dpy, 
			  XtWindow(_plotter->toplevel), new_cmap);
      xretval = XAllocColor (_plotter->dpy, new_cmap, rgb_ptr);
    }

  if (xretval == 0)
    /* allocation of new color cell failed */
    {
      if (!color_warning_printed)
	{
	  _plotter->warning ("color supply exhausted, can't create new colors");
	  color_warning_printed = true;
	}

      /* add null color cell to head of cache list */
      cptr = (Colorrecord *)_plot_xmalloc (sizeof (Colorrecord));
      cptr->rgb.red = rgb_red;
      cptr->rgb.green = rgb_green;
      cptr->rgb.blue = rgb_blue;
      cptr->allocated = false;
      cptr->frame = _plotter->frame_number; /* keep track of frame number */
      cptr->next = _plotter->x_colorlist;
      _plotter->x_colorlist = cptr;
      return false;
    }
  else
    /* allocation succeeded, add new color cell to head of cache list */
    {
      cptr = (Colorrecord *)_plot_xmalloc (sizeof (Colorrecord));
      memcpy (&(cptr->rgb), rgb_ptr, sizeof (XColor));
      /* be sure to include unquantized RGB values */
      cptr->rgb.red = rgb_red;
      cptr->rgb.green = rgb_green;
      cptr->rgb.blue = rgb_blue;
      cptr->allocated = true;
      cptr->frame = _plotter->frame_number; /* keep track of frame number */
      cptr->next = _plotter->x_colorlist;
      _plotter->x_colorlist = cptr;
      return true;
    }
}
