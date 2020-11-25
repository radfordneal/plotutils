/* This file contains the XDrawablePlotter (and XPlotter) version of the
   low-level paint_text_string() method, which is called to plot a label in
   the current (non-Hershey) font, at the current fontsize and textangle.
   The label is just a string: no control codes (font switching or
   sub/superscripts).  The width of the string in user units is returned.

   This version not does support center-justification and right
   justification; only the default left-justification.  That is all
   right, since justification is handled at a higher level. */

/* This file also contains the XDrawablePlotter (and XPlotter) version of
   the flabelwidth_other() method, which is called to compute the width, in
   user coordinates, of a label. */

#include "sys-defines.h"
#include "extern.h"

/* When this is called in g_alabel.c, the X font has already been
   retrieved, in whole or in part (by calling "_set_font()", which in turn
   calls "_plotter->retrieve_font()", which is bound to the
   _x_retrieve_font() routine in x_retrieve.c).  I.e., whatever portion of
   the X font was required to be retrieved in order to return font metrics,
   has previously been retrieved.

   To retrieve a larger part, we call _x_retrieve_font() again.  But this
   time, we pass the label to be rendered to _x_retrieve_font() as a
   "hint", i.e., as a data member of (the driver-specific part of) the
   drawing state.  That tells _x_retrieve_font how much more of the font to
   retrieve.  This scheme is a hack, but it works (and doesn't violate
   layering).

   The original retrieval may well have mapped the font name to something
   else (in the case of an unavailable X font, a default X font may have
   been substituted).  So we're careful to use drawstate->true_font_name as
   drawstate->font_name, for the duration of the second retrieval.  Note:
   this requires a strcpy, since _x_retrieve_font() may free the
   true_font_name member. */

double
#ifdef _HAVE_PROTOS
_x_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_x_paint_text_string (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;
     int v_just;
#endif
{
  const char *saved_font_name;
  char *temp_font_name;
  bool ok;
  double x, y, width;
  int offset = 0;
  
  /* sanity check; this routine supports only baseline positioning */
  if (v_just != JUST_BASE)
    return 0.0;

  /* similarly for horizontal justification */
  if (h_just != JUST_LEFT)
    return 0.0;

  if (*s == (unsigned char)'\0')
    return 0.0;

  /* Do retrieval, but use current `true_font_name' as our font name (see
     above; we've previously retrieved a subset of it). */

  if (_plotter->drawstate->true_font_name == NULL) /* shouldn't happen */
    return 0.0;

  saved_font_name = _plotter->drawstate->font_name;
  temp_font_name = 
    (char *)_plot_xmalloc (strlen (_plotter->drawstate->true_font_name) + 1);
  strcpy (temp_font_name, _plotter->drawstate->true_font_name);
  _plotter->drawstate->font_name = temp_font_name;

  _plotter->drawstate->x_label = s; /* pass label hint */
  ok = _x_retrieve_font (S___(_plotter));
  _plotter->drawstate->x_label = NULL; /* restore label hint to default */

  _plotter->drawstate->font_name = saved_font_name;
  free (temp_font_name);

  if (!ok)			/* shouldn't happen */
    return 0.0;

  /* Set font in GC used for drawing (the other GC, used for filling, is
     left alone).  _x_retrieve_font() does not do this. */
  XSetFont (_plotter->x_dpy, _plotter->drawstate->x_gc_fg,
	    _plotter->drawstate->x_font_struct->fid);

  /* select our pen color as foreground color in X GC used for drawing */
  _x_set_pen_color (S___(_plotter));
  
  /* compute position in device coordinates */
  x = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  y = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  
  if (_plotter->drawstate->x_native_positioning)
    /* a special case: the font name did not include a pixel matrix, or it
       did but the text rotation angle is zero; so move the easy way, i.e.,
       use native repositioning */
    {
      int label_len = strlen ((char *)s);
      int ix = IROUND(x);
      int iy = IROUND(y);

      /* X11 protocol OOB check */
      if (XOOB_INT(ix) || XOOB_INT(iy))
	{
	  _plotter->warning (R___(_plotter) 
	    "not drawing a text string that is positioned too far for X11");
	  return 0.0;
	}

      if (_plotter->x_double_buffering != DBL_NONE)
	/* double buffering, have a `x_drawable3' to draw into */
	XDrawString (_plotter->x_dpy, _plotter->x_drawable3, 
		     _plotter->drawstate->x_gc_fg, 
		     ix, iy, (char *)s, label_len);
      else
	{
	  /* not double buffering, have no `x_drawable3' */
	  if (_plotter->x_drawable1)
	    XDrawString (_plotter->x_dpy, _plotter->x_drawable1, 
			 _plotter->drawstate->x_gc_fg, 
			 ix, iy, (char *)s, label_len);
	  if (_plotter->x_drawable2)
	    XDrawString (_plotter->x_dpy, _plotter->x_drawable2, 
			 _plotter->drawstate->x_gc_fg, 
			 ix, iy, (char *)s, label_len);
	}

      /* compute width of string in normalized units (font size = 1000) */
      offset = IROUND(1000.0 * XTextWidth (_plotter->drawstate->x_font_struct, 
					   (char *)s, 
					   (int)(strlen((char *)s))) / 
		      _plotter->drawstate->x_font_pixmatrix[0]);
    }  
  else 
    /* general case: due to nonzero text rotation and/or a non-uniform
       transformation from user to device coordinates, a pixel matrix
       appeared explicitly in the font name (and hence the font name was an
       XLFD font name); must move the cursor and plot each character
       individually.  */
    {
      const unsigned char *stringptr = s;
      
      /* loop over characters */
      while (*stringptr)
	{
	  int charno = *stringptr;
	  int char_offset = 
	    charno - _plotter->drawstate->x_font_struct->min_char_or_byte2;
	  int ix, iy;
	  
	  ix = IROUND(x + 
		      offset * _plotter->drawstate->x_font_pixmatrix[0]/1000.0);
	  iy = IROUND(y 
		      -offset * _plotter->drawstate->x_font_pixmatrix[1]/1000.0);

	  /* X11 protocol OOB check */
	  if (XOOB_INT(ix) || XOOB_INT(iy))
	    {
	      _plotter->warning (R___(_plotter) 
	        "truncating a text string that extends too far for X11");
	      return 0.0;
	    }

	  if (_plotter->x_double_buffering != DBL_NONE)
	    /* double buffering, have a `x_drawable3' to draw into */
	    XDrawString (_plotter->x_dpy, _plotter->x_drawable3,
			 _plotter->drawstate->x_gc_fg, 
			 ix, iy, (char *)stringptr, 1);
	  else
	    /* not double buffering, have no `x_drawable3' */
	    {
	      if (_plotter->x_drawable1)
		XDrawString (_plotter->x_dpy, _plotter->x_drawable1,
			     _plotter->drawstate->x_gc_fg, 
			     ix, iy, (char *)stringptr, 1);
	      if (_plotter->x_drawable2)
		XDrawString (_plotter->x_dpy, _plotter->x_drawable2,
			     _plotter->drawstate->x_gc_fg, 
			     ix, iy, (char *)stringptr, 1);
	    }
	  
	  /* add this glyph's contribution to the width of the string, in
             normalized units (font size = 1000) */
	  offset += (_plotter->drawstate->x_font_struct->per_char ?
		     _plotter->drawstate->x_font_struct->per_char[char_offset].attributes :
		     _plotter->drawstate->x_font_struct->min_bounds.attributes);

	  stringptr++;
	}
    }
  
  /* convert normalized string width to width in user coors */
  width = _plotter->drawstate->true_font_size * (double)offset / 1000.0;

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return width;
}

/* Compute width, in user coordinates, of label in the currently selected
   font (no escape sequences!).  Current font is assumed to be a
   non-Hershey font (so we have an X font structure for it).  This is
   installed as an internal class method, invoked if the current font is
   non-Hershey (which means Postscript, PCL, or `other' [i.e. any non-PS,
   non-PCL, retrievable X font].  */

/* When this is called in g_alabel.c, the X font has already been
   retrieved, in whole or in part (by calling "_set_font()", which in turn
   calls "_plotter->retrieve_font()", i.e., which calls the
   _x_retrieve_font() routine in x_retrieve.c).  I.e., whatever portion of
   the X font was required to be retrieved in order to return font metrics,
   has previously been retrieved.

   To retrieve a larger part, we call _x_retrieve_font() again.  But this
   time, we pass the label to be rendered to _x_retrieve_font() as a
   "hint", i.e., as a data member of (the driver-specific part of) the
   drawing state.  That tells _x_retrieve_font how much more of the font to
   retrieve.  This scheme is an ugly hack, but it works (and doesn't
   violate layering).

   The original retrieval may well have mapped the font name to something
   else (in the case of an unavailable X font, a default X font may have
   been substituted).  So we're careful to use drawstate->true_font_name as
   drawstate->font_name, for the duration of the second retrieval.  Note:
   this requires a strcpy, since _x_retrieve_font() may free the
   true_font_name member. */

double
#ifdef _HAVE_PROTOS
_x_get_text_width (R___(Plotter *_plotter) const unsigned char *s)
#else
_x_get_text_width (R___(_plotter) s)
     S___(Plotter *_plotter;)
     const unsigned char *s;
#endif
{
  const char *saved_font_name;
  char *temp_font_name;
  bool ok;
  int offset = 0;
  double label_width;
  
  /* Do retrieval, but use current `true_font_name' as our font name (see
     above; we've previously retrieved a subset of it). */

  if (_plotter->drawstate->true_font_name == NULL) /* shouldn't happen */
    return 0.0;

  saved_font_name = _plotter->drawstate->font_name;
  temp_font_name = 
    (char *)_plot_xmalloc (strlen (_plotter->drawstate->true_font_name) + 1);
  strcpy (temp_font_name, _plotter->drawstate->true_font_name);
  _plotter->drawstate->font_name = temp_font_name;

  _plotter->drawstate->x_label = s; /* pass label hint */
  ok = _x_retrieve_font (S___(_plotter));
  _plotter->drawstate->x_label = NULL; /* restore label hint to default */

  _plotter->drawstate->font_name = saved_font_name;
  free (temp_font_name);

  if (!ok)			/* shouldn't happen */
    return 0.0;

  if (_plotter->drawstate->x_native_positioning)
    /* have a non-XLFD font, or an XLFD with zero textrotation, no shearing */

    /* compute width of string in normalized units (font size = 1000) */
    offset = IROUND(1000.0 * XTextWidth (_plotter->drawstate->x_font_struct, 
					 (char *)s, 
					 (int)(strlen((char *)s))) / 
		    _plotter->drawstate->x_font_pixmatrix[0]);
  else				
    /* necessarily have an XLFD font, may need to take shearing into account */
    {
      /* loop over characters */
      while (*s)
	{
	  int charno = *s;
	  int char_offset = 
	    charno - _plotter->drawstate->x_font_struct->min_char_or_byte2;
	  
	  /* add this glyph's contribution to the width of the string, in
             normalized units (font size = 1000) */
	  offset += (_plotter->drawstate->x_font_struct->per_char ?
		     _plotter->drawstate->x_font_struct->per_char[char_offset].attributes :
		     _plotter->drawstate->x_font_struct->min_bounds.attributes);
	  s++;
	}
    }

  /* convert normalized string width to width in user coors */
  label_width = _plotter->drawstate->true_font_size * (double)offset / 1000.0;

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return label_width;
}
