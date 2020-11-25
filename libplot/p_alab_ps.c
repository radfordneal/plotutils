/* This file contains the PS-driver-specific version of the low-level
   falabel_ps() method, which is called to plot a label in the current PS
   font, at the current fontsize and textangle.  The label is just a
   string: no control codes (font switching or sub/superscripts).

   The width of the string in user units is returned.  On exit, the
   graphics cursor position is repositioned to the end of the string.

   This version does not support center-justification and
   right-justification; only the default left-justification.  That is
   all right, since label justification is handled at a higher level. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

/* This prints a single-font, single-font-size label, and repositions to
   the end after printing.  When this is called, the current point is on
   the intended baseline of the label.  */

double
#ifdef _HAVE_PROTOS
_p_falabel_ps (const unsigned char *s, int h_just)
#else
_p_falabel_ps (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  int i, master_font_index;
  double width;
  unsigned char *ptr;
  double theta, costheta, sintheta;
  double norm;
  double crockshift_x, crockshift_y;
  double dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3;
  double font_ascent, font_descent, up, down;
  double user_font_size = _plotter->drawstate->true_font_size;
  double device_font_size;
  double user_text_transformation_matrix[6];
  double text_transformation_matrix[6];
  bool pcl_font;
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  if (h_just != JUST_LEFT)
    {
      _plotter->warning ("ignoring request to use non-default justification for a label");
      return 0.0;
    }

  /* sanity check */
#ifndef USE_LJ_FONTS
  if (_plotter->drawstate->font_type != F_POSTSCRIPT)
    return 0.0;
#else  /* USE_LJ_FONTS */
  if (_plotter->drawstate->font_type != F_POSTSCRIPT
      && _plotter->drawstate->font_type != F_PCL)
    return 0.0;
#endif
  pcl_font = (_plotter->drawstate->font_type == F_PCL ? true : false);

  /* compute index of font in master table of PS [or PCL] fonts, in g_fontdb.c */
  if (pcl_font)
    master_font_index =
      (_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  else				/* one of the 35 standard PS fonts */
    master_font_index =
      (_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* font ascent and descent (taken from the font's bounding box) */
  if (pcl_font)
    {
      font_ascent = (double)((_pcl_font_info[master_font_index]).font_ascent);
      font_descent = (double)((_pcl_font_info[master_font_index]).font_descent);
    }
  else				/* PS font */
    {
      font_ascent = (double)((_ps_font_info[master_font_index]).font_ascent);
      font_descent = (double)((_ps_font_info[master_font_index]).font_descent);
    }
  up = user_font_size * font_ascent / 1000.0;
  down = user_font_size * font_descent / 1000.0;

  /* Current point is on the baseline, but the rendering logic of the PS
     code in the idraw prologue requires us to perform a vertical shift at
     this point.  (We'll undo the vertical shift immediately.) */
  _plotter->drawstate->pos.x -= (user_font_size - down) * sintheta;
  _plotter->drawstate->pos.y += (user_font_size - down) * costheta;

  /* the idraw PS prologue (see p_header.c) performs an additional
     [gratuitous] vertical shift by 1 unit, which we must compensate for */
  {
    double ctm_norm = _matrix_norm (_plotter->drawstate->transform.m);
    
    crockshift_x = sintheta / ctm_norm;
    crockshift_y = costheta / ctm_norm;
  }
  _plotter->drawstate->pos.x += crockshift_x;
  _plotter->drawstate->pos.y -= crockshift_y;

  /* this transformation matrix rotates, and translates; it maps (0,0) to
     the origin of the string, in user coordinates */
  user_text_transformation_matrix[0] = costheta;
  user_text_transformation_matrix[1] = sintheta;
  user_text_transformation_matrix[2] = - sintheta;
  user_text_transformation_matrix[3] = costheta;
  user_text_transformation_matrix[4] = _plotter->drawstate->pos.x;
  user_text_transformation_matrix[5] = _plotter->drawstate->pos.y;

  /* undo vertical shifts performed above */
  _plotter->drawstate->pos.x += (user_font_size - down) * sintheta;
  _plotter->drawstate->pos.y -= (user_font_size - down) * costheta;
  _plotter->drawstate->pos.x -= crockshift_x;
  _plotter->drawstate->pos.y += crockshift_y;

  /* Construct a temporary matrix that rotates, translates, and then maps
     to device coordinates.  This matrix transforms from a frame in which
     nominal character sizes are roughly 1 unit in the horizontal and
     vertical directions, to device coordinates. */
  _matrix_product (user_text_transformation_matrix, 
		   _plotter->drawstate->transform.m,
		   text_transformation_matrix);

  /* We need to extract a quantity we can call a font size in device
     coordinates, for the benefit of idraw.  (Idraw needs to retrieve an X
     font, and scale it.  Idraw does not make use of modern [X11R6+] font
     scaling technology; it does its own scaling of bitmaps.)

     We define this to be user_font_size (the nominal font size in user
     coordinates), times the norm of the linear tranformation contained in
     the temporary matrix we just constructed (the magnitude of its larger
     singular value).  Recall that for any square matrix M, the singular
     values are the square roots of the eigenvalues of the symmetric matrix
     M^t M. */

  norm = _matrix_norm (text_transformation_matrix);

  if (norm == 0.0)		/* avoid division by zero */
    return 0.0;

  device_font_size = norm * user_font_size;

  /* Now scale the text transformation matrix so that the linear
     transformation contained in it has unit norm (if there is no shearing,
     it will just be a rotation; if there is no rotation either, it will be
     the identity matrix. */
  for (i = 0; i < 4; i++)
    text_transformation_matrix[i] /= norm;

  /* prologue instruction, plus idraw directive: start of Text */
  strcpy (_plotter->page->point, "Begin %I Text\n");
  _update_buffer (_plotter->page);

  /* idraw directive, plus prologue instruction: set foreground color */
  _plotter->set_pen_color();	/* invoked lazily, i.e. when needed */
  sprintf (_plotter->page->point, "%%I cfg %s\n%g %g %g SetCFg\n",
	   _idraw_stdcolornames[_plotter->drawstate->idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red,
	   _plotter->drawstate->ps_fgcolor_green,
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer (_plotter->page);

  /* idraw directive: X Windows font name, which incorporates the X font
     size.  We use our primary X font name (the `x_name' field, not the
     `x_name_alt' field if any). */

  /* N.B. this directive sets the _pixel_ size of the X font to be our
     current point size.  That would really be appropriate only if the
     screen resolution is 72 dpi.  But idraw seems to prefer setting the
     pixel size to setting the point size. */

  if (pcl_font)			/* one of the 45 PCL fonts */
    {
      const char *ps_name;
      
      /* this is to support the Tidbits-is-Wingdings botch */
      if (_pcl_font_info[master_font_index].substitute_ps_name)
	ps_name = _pcl_font_info[master_font_index].substitute_ps_name;
      else
	ps_name = _pcl_font_info[master_font_index].ps_name;

      sprintf (_plotter->page->point,
	       "%%I f -*-%s-*-%d-*-*-*-*-*-*-*\n", 
	       (_pcl_font_info[master_font_index]).x_name, 
	       IROUND(device_font_size));
      _update_buffer (_plotter->page);

      /* prolog instruction: PS font name and size */
      sprintf (_plotter->page->point, "/%s %f SetF\n", 
	       ps_name,
	       device_font_size);
      _update_buffer (_plotter->page);
    }
  else				/* one of the 35 PS fonts */
    {
      sprintf (_plotter->page->point,
	       "%%I f -*-%s-*-%d-*-*-*-*-*-*-*\n", 
	       (_ps_font_info[master_font_index]).x_name, 
	       IROUND(device_font_size));
      _update_buffer (_plotter->page);

      /* prolog instruction: PS font name and size */
      sprintf (_plotter->page->point, "/%s %f SetF\n", 
	       _ps_font_info[master_font_index].ps_name,
	       device_font_size);
      _update_buffer (_plotter->page);
    }

  /* idraw directive and prologue instruction: text transformation matrix */
  strcpy (_plotter->page->point, "%I t\n[ ");
  _update_buffer (_plotter->page);

  for (i = 0; i < 6; i++)
    {
      sprintf (_plotter->page->point, "%.7g ", 
	       text_transformation_matrix[i]);
      _update_buffer (_plotter->page);      
    }
  
  /* width of the substring in user units (used below in constructing a
     bounding box, and in performing repositioning at end) */
  if (pcl_font)
    width = _plotter->flabelwidth_pcl (s);
  else
    width = _plotter->flabelwidth_ps (s);

  /* to compute an EPS-style bounding box, first compute offsets to the
     four vertices of the smallest rectangle containing the string */
  dx0 = - sintheta * (-down);
  dy0 =   costheta * (-down);
  
  dx1 = - sintheta * up;
  dy1 =   costheta * up;
  
  dx2 = costheta * width - sintheta * (-down);
  dy2 = sintheta * width + costheta * (-down);
  
  dx3 = costheta * width - sintheta * up;
  dy3 = sintheta * width + costheta * up;

  /* record that we're using all four vertices (args of _set_range() are in
     device units, not user units) */
  _set_range (_plotter->page, XD ((_plotter->drawstate->pos).x + dx0, (_plotter->drawstate->pos).y + dy0),
	      YD ((_plotter->drawstate->pos).x + dx0, (_plotter->drawstate->pos).y + dy0));
  _set_range (_plotter->page, XD ((_plotter->drawstate->pos).x + dx1, (_plotter->drawstate->pos).y + dy1), 
	      YD ((_plotter->drawstate->pos).x + dx1, (_plotter->drawstate->pos).y + dy1));
  _set_range (_plotter->page, XD ((_plotter->drawstate->pos).x + dx2, (_plotter->drawstate->pos).y + dy2), 
	      YD ((_plotter->drawstate->pos).x + dx2, (_plotter->drawstate->pos).y + dy2));
  _set_range (_plotter->page, XD ((_plotter->drawstate->pos).x + dx3, (_plotter->drawstate->pos).y + dy3), 
	      YD ((_plotter->drawstate->pos).x + dx3, (_plotter->drawstate->pos).y + dy3));

  /* Finish outputting transformation matrix; begin outputting string. */
  /* Escape all backslashes etc. in the text string, before output. */
  strcpy (_plotter->page->point, " ] concat\n\
%I\n\
[\n\
(");
  _update_buffer (_plotter->page);

  ptr = (unsigned char *)_plotter->page->point;
  while (*s)
    {
      switch (*s)
	{
	case '(':		/* for PS, escape ()/ */
	case ')':
	case '\\':
	  *ptr++ = (unsigned char)'\\';
	  *ptr++ = *s++;
          break;
	default:
          if GOOD_PRINTABLE_ASCII (*s)
	    *ptr++ = *s++;
          else
            {	    
               sprintf ((char *)ptr, "\\%03o", (unsigned int)*s);
               ptr += 4;
               s++;
            }
          break;
	}
    }
  *ptr = (unsigned char)'\0';
  _update_buffer (_plotter->page);

  /* prologue instruction: end of text */
  strcpy (_plotter->page->point, ")\n\
] Text\n\
End\n\
\n");
  _update_buffer (_plotter->page);

  /* reposition after printing substring */
  _plotter->drawstate->pos.x += costheta * width;
  _plotter->drawstate->pos.y += sintheta * width;

  /* flag current PS or PCL font as used */
#ifdef USE_LJ_FONTS
  if (pcl_font)
    _plotter->pcl_font_used[master_font_index] = true;

  else
    _plotter->ps_font_used[master_font_index] = true;
#else
    _plotter->ps_font_used[master_font_index] = true;
#endif

  return width;
}
