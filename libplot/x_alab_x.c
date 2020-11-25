/* This file contains the X11-driver-specific version of the low-level
   falabel_other() method, which is called to plot a label in the current
   font, at the current fontsize and textangle.  The label is just a
   string: no control codes (font switching or sub/superscripts).

   As the name suggests, falabel_other was written for `other' X fonts
   (user-specified ones not listed in our font database in g_fontdb.h).
   Since the rendering method is the same for all X fonts, we invoke this
   method for PS fonts that appear there, too.

   The width of the string in user units is returned.  On exit, the
   graphics cursor position is repositioned to the end of the string.

   This version not does support center-justification and right
   justification; only the default left-justification.  That is all
   right, since justification is handled at a higher level. */

/* This file also contains the X11-driver-specific version of the
   flabelwidth_other() method, which is called to compute the width, in
   user coordinates, of a label. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

static bool _suppress_retrieve = false;	/* avoids unnecessary font retrieval */

/* This prints a single-font, single-font-size label, and repositions to
   the end after printing.  When this is called, the current point is on
   the intended baseline of the label.  */

double
#ifdef _HAVE_PROTOS
_x_falabel_other (const unsigned char *s, int h_just)
#else
_x_falabel_other (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  double x, y, width;
  double theta, costheta, sintheta;
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  if (h_just != JUST_LEFT)
    {
      _plotter->warning ("ignoring request to use non-default justification for a label");
      return 0.0;
    }

  /* select our pen color as foreground color in X GC used for drawing */
  _plotter->set_pen_color();
  
  /* compute position in device coordinates */
  x = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  y = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  
  /* Retrieve the font -- all of it that we'll need.  We may have
     previously retrieved only an empty subset of it.  See x_retrieve.c for
     information on character subsetting. */
  _plotter->x_label = s;	/* pass hint */
  _plotter->retrieve_font ();
  _plotter->x_label = NULL;

  /* Set font in GC used for drawing (the other GC, used for filling, is
     left alone).  _x_retrieve_font() does not do this. */
  XSetFont (_plotter->dpy, _plotter->drawstate->gc_fg,
	    _plotter->drawstate->x_font_struct->fid);

  if (_plotter->drawstate->native_positioning)
    {
      /* a special case: the font name did not include a pixel matrix, or
         it did but the text rotation angle is zero; so move the easy way,
         i.e., use native repositioning */
      int label_len = strlen ((char *)s);
      int ix = IROUND(x);
      int iy = IROUND(y);

      /* X11 protocol OOB check */
      if (XOOB_INT(ix) || XOOB_INT(iy))
	return 0.0;

      if (_plotter->double_buffering != DBL_NONE)
	XDrawString (_plotter->dpy, _plotter->drawable3, 
		     _plotter->drawstate->gc_fg, 
		     ix, iy, (char *)s, label_len);
      else
	{
	  if (_plotter->drawable1)
	    XDrawString (_plotter->dpy, _plotter->drawable1, 
			 _plotter->drawstate->gc_fg, 
			 ix, iy, (char *)s, label_len);
	  if (_plotter->drawable2)
	    XDrawString (_plotter->dpy, _plotter->drawable2, 
			 _plotter->drawstate->gc_fg, 
			 ix, iy, (char *)s, label_len);
	}
    }  
  else 
    {
      /* general case: due to nonzero text rotation and/or a non-uniform
	 transformation from user to device coordinates, a pixel matrix
	 appeared explicitly in the font name (and hence the font name was
	 an XLFD font name); must move the cursor and plot each character
	 individually.  */
      double offset = 0.0;
      const unsigned char *stringptr = s;
      
      while (*stringptr)
	{
	  int charno = *stringptr;
	  int char_metric_offset = 
	    charno - _plotter->drawstate->x_font_struct->min_char_or_byte2;
	  int ix, iy;
	  
	  ix = IROUND(x + 
		      offset * _plotter->drawstate->font_pixmatrix[0]/1000.0);
	  iy = IROUND(y 
		      -offset * _plotter->drawstate->font_pixmatrix[1]/1000.0);

	  /* X11 protocol OOB check */
	  if (XOOB_INT(ix) || XOOB_INT(iy))
	    return 0.0;

	  if (_plotter->double_buffering != DBL_NONE)
	    XDrawString (_plotter->dpy, _plotter->drawable3,
			 _plotter->drawstate->gc_fg, 
			 ix, iy, (char *)stringptr, 1);
	  else
	    {
	      if (_plotter->drawable1)
		XDrawString (_plotter->dpy, _plotter->drawable1,
			     _plotter->drawstate->gc_fg, 
			     ix, iy, (char *)stringptr, 1);
	      if (_plotter->drawable2)
		XDrawString (_plotter->dpy, _plotter->drawable2,
			     _plotter->drawstate->gc_fg, 
			     ix, iy, (char *)stringptr, 1);
	    }
	  
	  stringptr++;
	  offset += (double)(_plotter->drawstate->x_font_struct->per_char ?
			     _plotter->drawstate->x_font_struct->per_char[char_metric_offset].attributes :
			     _plotter->drawstate->x_font_struct->min_bounds.attributes);
	}
    }
  
  /* width of the substring in user units */
  _suppress_retrieve = true;	/* avoid unnecessary retrieval of X font */
  width = _x_flabelwidth_other (s);
  _suppress_retrieve = false;

  /* label rotation angle in radians */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* reposition after printing substring */
  _plotter->drawstate->pos.x += costheta * width;
  _plotter->drawstate->pos.y += sintheta * width;

  _handle_x_events();

  return width;
}

/* Compute width, in user coordinates, of label in the currently selected
   font (no escape sequences!).  Current font is assumed to be a
   non-Hershey font (so we have an X font structure for it).  This is
   installed as an internal class method, invoked if the current font is
   non-Hershey (which means Postscript, PCL, or `other' [i.e. any non-PS,
   non-PCL, retrievable X font].  */

double
#ifdef _HAVE_PROTOS
_x_flabelwidth_other (const unsigned char *s)
#else
_x_flabelwidth_other (s)
     const unsigned char *s;
#endif
{
  int offset = 0;
  double label_width;
  
  /* Retrieve the font -- all of it that we'll need.  We may have
     previously retrieved only an empty subset of it.  See x_retrieve.c for
     information on character subsetting. */
  if (_suppress_retrieve == false)
    /* retrieve X font */
    {
      _plotter->x_label = s;	/* pass hint */
      _plotter->retrieve_font ();
      _plotter->x_label = NULL;
    }

  if (_plotter->drawstate->native_positioning)
    /* have a non-XLFD font, or an XLFD with zero textrotation, no shearing */
    offset = IROUND(1000.0 * XTextWidth (_plotter->drawstate->x_font_struct, 
					 (char *)s, 
					 (int)(strlen((char *)s))) / 
		    _plotter->drawstate->font_pixmatrix[0]);
  else				
    /* necessarily have an XLFD font, may need to take shearing into account */
    {
      while (*s)
	{
	  int charno = *s;
	  int char_metric_offset = 
	    charno - _plotter->drawstate->x_font_struct->min_char_or_byte2;
	  
	  offset += (_plotter->drawstate->x_font_struct->per_char ?
		     _plotter->drawstate->x_font_struct->per_char[char_metric_offset].attributes :
		     _plotter->drawstate->x_font_struct->min_bounds.attributes);
	  s++;
	}
    }

  /* multiply normalized width by current font size in user coors */
  label_width = _plotter->drawstate->true_font_size * (double)offset / 1000.0;

  _handle_x_events();

  return label_width;
}
