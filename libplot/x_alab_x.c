/* This file contains the XDrawablePlotter (and XPlotter) version of the
   low-level falabel_other() method, which is called to plot a label in the
   current font, at the current fontsize and textangle.  The label is just
   a string: no control codes (font switching or sub/superscripts).

   As the name suggests, falabel_other was written for `other' X fonts
   (user-specified ones not listed in our font database in g_fontdb.h).
   Since the rendering method is the same for all X fonts, we invoke this
   method for PS fonts that appear there, too.

   The width of the string in user units is returned.  On exit, the
   graphics cursor position is repositioned to the end of the string.

   This version not does support center-justification and right
   justification; only the default left-justification.  That is all
   right, since justification is handled at a higher level. */

/* This file also contains the XDrawablePlotter (and XPlotter) version of
   the flabelwidth_other() method, which is called to compute the width, in
   user coordinates, of a label. */

#include "sys-defines.h"
#include "extern.h"

/* When this is called (by calling "_plotter->falabel_other()"), the X font
   may already been retrieved, in whole or in part (by calling
   "_plotter->retrieve_font()", i.e., by calling the _x_retrieve_font()
   subroutine in x_retrieve.c).  See g_alabel.c, where the calling takes
   place.  To retrieve a larger part, the label needs to be passed to
   _x_retrieve_font() as a "hint".  That is what is done in this
   function. */

double
#ifdef _HAVE_PROTOS
_x_falabel_other (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_x_falabel_other (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
     int v_just;  /* vertical justification: JUST_TOP, HALF, BASE, BOTTOM */
#endif
{
  double x, y, width;
  double theta, costheta, sintheta;
  int offset = 0;
  
  /* sanity check; this routine supports only baseline positioning */
  if (v_just != JUST_BASE)
    return 0.0;

  /* similarly for horizontal justification */
  if (h_just != JUST_LEFT)
    {
      _plotter->warning (R___(_plotter) 
			 "ignoring request to use non-default justification for a label");
      return 0.0;
    }

  if (*s == (unsigned char)'\0')
    return 0.0;

  /* select our pen color as foreground color in X GC used for drawing */
  _plotter->set_pen_color (S___(_plotter));
  
  /* compute position in device coordinates */
  x = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  y = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  
  /* Retrieve the font -- all of it that we'll need, anyway.  We may have
     previously retrieved a proper subset of it (see above).  See
     x_retrieve.c for information on character subsetting. */
  _plotter->drawstate->x_label = s;	/* pass hint */
  _plotter->retrieve_font (S___(_plotter));
  _plotter->drawstate->x_label = NULL; /* restore to default value */

  /* Set font in GC used for drawing (the other GC, used for filling, is
     left alone).  _x_retrieve_font() does not do this. */
  XSetFont (_plotter->x_dpy, _plotter->drawstate->x_gc_fg,
	    _plotter->drawstate->x_font_struct->fid);

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
	return 0.0;

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
	  int char_metric_offset = 
	    charno - _plotter->drawstate->x_font_struct->min_char_or_byte2;
	  int ix, iy;
	  
	  ix = IROUND(x + 
		      offset * _plotter->drawstate->x_font_pixmatrix[0]/1000.0);
	  iy = IROUND(y 
		      -offset * _plotter->drawstate->x_font_pixmatrix[1]/1000.0);

	  /* X11 protocol OOB check */
	  if (XOOB_INT(ix) || XOOB_INT(iy))
	    return 0.0;

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
		     _plotter->drawstate->x_font_struct->per_char[char_metric_offset].attributes :
		     _plotter->drawstate->x_font_struct->min_bounds.attributes);

	  stringptr++;
	}
    }
  
  /* convert normalized string width to width in user coors */
  width = _plotter->drawstate->true_font_size * (double)offset / 1000.0;

  /* label rotation angle in radians */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* reposition after printing substring */
  _plotter->drawstate->pos.x += costheta * width;
  _plotter->drawstate->pos.y += sintheta * width;

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return width;
}

/* Counterparts of the preceding, for PS and PCL fonts.  If used by
   an X [Drawable] Plotter, they simply invoke the preceding. */

double
#ifdef _HAVE_PROTOS
_x_falabel_ps (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_x_falabel_ps (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
     int v_just;  /* vertical justification: JUST_TOP, HALF, BASE, BOTTOM */
#endif
{
  return _x_falabel_other (R___(_plotter) s, h_just, v_just);
}

double
#ifdef _HAVE_PROTOS
_x_falabel_pcl (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_x_falabel_pcl (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
     int v_just;  /* vertical justification: JUST_TOP, HALF, BASE, BOTTOM */
#endif
{
  return _x_falabel_other (R___(_plotter) s, h_just, v_just);
}

/* Compute width, in user coordinates, of label in the currently selected
   font (no escape sequences!).  Current font is assumed to be a
   non-Hershey font (so we have an X font structure for it).  This is
   installed as an internal class method, invoked if the current font is
   non-Hershey (which means Postscript, PCL, or `other' [i.e. any non-PS,
   non-PCL, retrievable X font].  */

double
#ifdef _HAVE_PROTOS
_x_flabelwidth_other (R___(Plotter *_plotter) const unsigned char *s)
#else
_x_flabelwidth_other (R___(_plotter) s)
     S___(Plotter *_plotter;)
     const unsigned char *s;
#endif
{
  int offset = 0;
  double label_width;
  
  /* Retrieve the font -- and retrieve only the portion that we'll need,
     which we specify by passing a "hint".  We may have previously
     retrieved the font, or a proper subset of it.  See x_retrieve.c for
     information on character subsetting. */
  _plotter->drawstate->x_label = s;	/* pass hint */
  _plotter->retrieve_font (S___(_plotter));
  _plotter->drawstate->x_label = NULL; /* restore to default value */

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
	  int char_metric_offset = 
	    charno - _plotter->drawstate->x_font_struct->min_char_or_byte2;
	  
	  /* add this glyph's contribution to the width of the string, in
             normalized units (font size = 1000) */
	  offset += (_plotter->drawstate->x_font_struct->per_char ?
		     _plotter->drawstate->x_font_struct->per_char[char_metric_offset].attributes :
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

/* Counterparts of the preceding, for PS and PCL fonts.  If used by an
   X [Drawable] Plotter, they simply invoke the preceding. */

double
#ifdef _HAVE_PROTOS
_x_flabelwidth_ps (R___(Plotter *_plotter) const unsigned char *s)
#else
_x_flabelwidth_ps (R___(_plotter) s)
     S___(Plotter *_plotter;)
     const unsigned char *s;
#endif
{
  return _x_flabelwidth_other (R___(_plotter) s);
}

double
#ifdef _HAVE_PROTOS
_x_flabelwidth_pcl (R___(Plotter *_plotter) const unsigned char *s)
#else
_x_flabelwidth_pcl (R___(_plotter) s)
     S___(Plotter *_plotter;)
     const unsigned char *s;
#endif
{
  return _x_flabelwidth_other (R___(_plotter) s);
}
