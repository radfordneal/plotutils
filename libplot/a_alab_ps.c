/* This file contains the AI-driver-specific version of the low-level
   falabel_ps() method, which is called to plot a label in the current font
   (either PS or PCL), at the current fontsize and textangle.  The label is
   just a string: no control codes (font switching or sub/superscripts).

   The width of the string in user units is returned.  On exit, the
   graphics cursor position is repositioned to the end of the string. */

#include "sys-defines.h"
#include "extern.h"

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

/* This prints a single-font, single-font-size label, and repositions to
   the end after printing.  When this is called, the current point is on
   the intended baseline of the label.  */

double
#ifdef _HAVE_PROTOS
_a_falabel_ps (const unsigned char *s, int h_just)
#else
_a_falabel_ps (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  int i, master_font_index;
  int justify_code;
  double width;
  unsigned char *ptr;
  double theta, costheta, sintheta;
  double norm;
  double dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3;
  double font_ascent, font_descent, up, down;
  double user_font_size = _plotter->drawstate->true_font_size;
  double device_font_size;
  double user_text_transformation_matrix[6];
  double text_transformation_matrix[6];
  double lshift;
  bool pcl_font;
  Color old_fillcolor;
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* sanity check */
  if (_plotter->drawstate->font_type != F_POSTSCRIPT
      && _plotter->drawstate->font_type != F_PCL)
    return 0.0;
  pcl_font = (_plotter->drawstate->font_type == F_PCL ? true : false);

  /* compute index of font in master table of PS [or PCL] fonts, in g_fontdb.c */
  if (pcl_font)			/* one of the 45 standard PCL fonts */
    master_font_index =
      (_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  else				/* one of the 35 standard PS fonts */
    master_font_index =
      (_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

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

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* this transformation matrix rotates, and translates; it maps (0,0) to
     the origin of the string, in user coordinates */
  user_text_transformation_matrix[0] = costheta;
  user_text_transformation_matrix[1] = sintheta;
  user_text_transformation_matrix[2] = - sintheta;
  user_text_transformation_matrix[3] = costheta;
  user_text_transformation_matrix[4] = _plotter->drawstate->pos.x;
  user_text_transformation_matrix[5] = _plotter->drawstate->pos.y;

  /* Construct a temporary matrix that rotates, translates, and then maps
     to device coordinates.  This matrix transforms from a frame in which
     nominal character sizes are roughly 1 unit in the horizontal and
     vertical directions, to device coordinates. */
  _matrix_product (user_text_transformation_matrix, 
		   _plotter->drawstate->transform.m,
		   text_transformation_matrix);

  /* We need to extract a quantity we can call a font size in device
     coordinates, for the benefit of AI.  (AI needs to retrieve a font, and
     transform it.)

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
     the identity matrix). */
  for (i = 0; i < 4; i++)
    text_transformation_matrix[i] /= norm;

  /* AI directive: begin `point text' object */
  strcpy (_plotter->page->point, "0 To\n");
  _update_buffer (_plotter->page);

  /* output text transformation matrix */
  for (i = 0; i < 6; i++)
    {
      sprintf (_plotter->page->point, "%.4f ", 
	       text_transformation_matrix[i]);
      _update_buffer (_plotter->page);      
    }
  strcpy (_plotter->page->point, "0 Tp\nTP\n");
  _update_buffer (_plotter->page);
  
  /* set render mode: fill text, rather than several other possibilities */
  strcpy (_plotter->page->point, "0 Tr\n");
  _update_buffer (_plotter->page);

  /* temporarily set AI's fill color to be the same as libplot's notion of
     pen color (since letters in label will be drawn as filled outlines) */
  old_fillcolor = _plotter->drawstate->fillcolor;
  _plotter->drawstate->fillcolor = _plotter->drawstate->fgcolor;
  _plotter->set_fill_color();	/* emit AI directive */
  _plotter->drawstate->fillcolor = old_fillcolor;

  /* set AI's pen color also, in particular set it to be the same as
     libplot's notion of pen color (even though we'll be filling, not
     stroking); this is a convenience for AI users who may wish e.g. to
     switch from filling letter outlines to stroking them */
  _plotter->set_pen_color();	/* emit AI directive */

  /* AI directive: set font name and size */
  {
    const char *ps_name;

    if (pcl_font)			/* one of the 45 PCL fonts */
      ps_name = _pcl_font_info[master_font_index].ps_name;
    else				/* one of the 35 PS fonts */
      ps_name = _ps_font_info[master_font_index].ps_name;
    
    /* specify font name (underscore indicates reencoding), font size */
    sprintf (_plotter->page->point, "/_%s %.4f Tf\n", 
	     ps_name, device_font_size);
    _update_buffer (_plotter->page);
  }
  
  /* set line horizontal expansion factor, in percent */
  strcpy (_plotter->page->point, "100 Tz\n");
  _update_buffer (_plotter->page);

  /* NO track kerning, please */
  strcpy (_plotter->page->point, "0 Tt\n");
  _update_buffer (_plotter->page);

  /* turn off pairwise kerning (currently, a libplot convention) */
  strcpy (_plotter->page->point, "0 TA\n");
  _update_buffer (_plotter->page);

  /* turn off ALL inter-character spacing */
  strcpy (_plotter->page->point, "0 0 0 TC\n");
  _update_buffer (_plotter->page);

  /* use the default inter-word spacing; no more, no less */
  strcpy (_plotter->page->point, "100 100 100 TW\n");
  _update_buffer (_plotter->page);

  /* no indentation at beginning of `paragraphs' */
  strcpy (_plotter->page->point, "0 0 0 Ti\n");
  _update_buffer (_plotter->page);

  /* specify justification */
  switch (h_just)
    {
    case JUST_LEFT:
    default:
      justify_code = 0;
      break;
    case JUST_CENTER:
      justify_code = 1;
      break;
    case JUST_RIGHT:
      justify_code = 2;
      break;
    }
  sprintf (_plotter->page->point, "%d Ta\n", justify_code);
  _update_buffer (_plotter->page);

  /* no hanging quotation marks */
  strcpy (_plotter->page->point, "0 Tq\n");
  _update_buffer (_plotter->page);

  /* no leading between lines of a paragraph or between paragraphs */
  strcpy (_plotter->page->point, "0 0 Tl\n");
  _update_buffer (_plotter->page);

  /* compute width of the substring in user units (used below in
     constructing a bounding box, and in performing repositioning at end) */
  if (pcl_font)
    width = _plotter->flabelwidth_pcl (s);
  else
    width = _plotter->flabelwidth_ps (s);

  /* for computing bounding box, compute justification-dependent leftward
     shift, as fraction of label width */
  switch (h_just)
    {
    case JUST_LEFT:
    default:
      lshift = 0.0;
      break;
    case JUST_CENTER:
      lshift = 0.5;
      break;
    case JUST_RIGHT:
      lshift = 1.0;
      break;
    }

  /* to compute an EPS-style bounding box, first compute offsets to the
     four vertices of the smallest rectangle containing the string */

  dx0 = costheta * (- lshift) * width - sintheta * (-down);
  dy0 = sintheta * (- lshift) * width + costheta * (-down);
  
  dx1 = costheta * (- lshift) * width - sintheta * up;
  dy1 = sintheta * (- lshift) * width + costheta * up;
  
  dx2 = costheta * (1.0 - lshift) * width - sintheta * (-down);
  dy2 = sintheta * (1.0 - lshift) * width + costheta * (-down);
  
  dx3 = costheta * (1.0 - lshift) * width - sintheta * up;
  dy3 = sintheta * (1.0 - lshift) * width + costheta * up;

  /* record that we're using all four vertices (args of _update_bbox() are in
     device units, not user units) */
  _update_bbox (_plotter->page, XD ((_plotter->drawstate->pos).x + dx0, (_plotter->drawstate->pos).y + dy0),
	      YD ((_plotter->drawstate->pos).x + dx0, (_plotter->drawstate->pos).y + dy0));
  _update_bbox (_plotter->page, XD ((_plotter->drawstate->pos).x + dx1, (_plotter->drawstate->pos).y + dy1), 
	      YD ((_plotter->drawstate->pos).x + dx1, (_plotter->drawstate->pos).y + dy1));
  _update_bbox (_plotter->page, XD ((_plotter->drawstate->pos).x + dx2, (_plotter->drawstate->pos).y + dy2), 
	      YD ((_plotter->drawstate->pos).x + dx2, (_plotter->drawstate->pos).y + dy2));
  _update_bbox (_plotter->page, XD ((_plotter->drawstate->pos).x + dx3, (_plotter->drawstate->pos).y + dy3), 
	      YD ((_plotter->drawstate->pos).x + dx3, (_plotter->drawstate->pos).y + dy3));

  /* output string as a PS string (i.e. surrounded by parentheses) */
  ptr = (unsigned char *)_plotter->page->point;
  *ptr++ = '(';
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
  *ptr++ = ')';
  *ptr = (unsigned char)'\0';
  _update_buffer (_plotter->page);

  /* AI directive: this is the text to be rendered */
  strcpy (_plotter->page->point, " Tx\n");
  _update_buffer (_plotter->page);

  /* AI directive: end of text object */
  strcpy (_plotter->page->point, "TO\n");
  _update_buffer (_plotter->page);

  /* reposition after printing string */
  _plotter->drawstate->pos.x += costheta * width;
  _plotter->drawstate->pos.y += sintheta * width;

  /* flag current PS or PCL font as used */
  if (pcl_font)
    _plotter->page->pcl_font_used[master_font_index] = true;
  else
    _plotter->page->ps_font_used[master_font_index] = true;

  return width;
}

/* Counterpart of the preceding, for PCL fonts.  If used by the AI Plotter,
   it simply invokes the preceding, which contains PCL font support. */
double
#ifdef _HAVE_PROTOS
_a_falabel_pcl (const unsigned char *s, int h_just)
#else
_a_falabel_pcl (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  return _a_falabel_ps (s, h_just);
}
