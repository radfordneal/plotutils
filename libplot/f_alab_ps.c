/* This file contains the Fig-driver-specific version of the low-level
   falabel_ps() method, which is called to plot a label in the current PS
   font, at the current fontsize and textangle.  The label is just a
   string: no control codes (font switching or sub/superscripts).

   The width of the string in user units is returned.  On exit, the
   graphics cursor position is repositioned to the end of the string.

   This version does not support `sheared' fonts, since xfig does not
   support them.  But it does support center-justification and right
   justification as well as the default left-justification, since the xfig
   format supports them. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define FONT_TYPE_LATEX 0	/* don't support LaTeX fonts currently */
#define FONT_TYPE_PS 4

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

/* This prints a single-font, single-font-size label, and repositions to
   the end after printing.  When this is called, the current point is on
   the intended baseline of the label.  */

/* Repositioning takes place after printing.  If string is left-justified,
   the motion is to the right; if it is right-justified, the motion is to
   the left.  If it is center-justified, there is no repositioning.  */

double
#ifdef _HAVE_PROTOS
_f_falabel_ps (const unsigned char *s, int h_just)
#else
_f_falabel_ps (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  int len, master_font_index;
  unsigned char *ptr, *t;
  double theta, costheta, sintheta;
  double label_width, label_ascent;
  double dx, translation_x, translation_y;
  double initial_x, initial_y;
  double horizontal_x, horizontal_y, vertical_x, vertical_y;
  double horizontal_fig_length, vertical_fig_length;
  double horizontal_fig_x, vertical_fig_x;
  double horizontal_fig_y, vertical_fig_y;
  double angle_device;
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* label rotation angle in radians */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* compute index of font in master table of PS fonts, in g_fontdb.h */
  master_font_index =
    (_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* compute label height and width, in user units */
  label_width = _plotter->flabelwidth_ps (s);
  label_ascent  = _plotter->drawstate->true_font_size * (_ps_font_info[master_font_index]).font_ascent / 1000.0;
  
  /* vector along which we'll move when we print the label (user frame) */
  switch (h_just)
    {
    case JUST_LEFT:
    default:
      dx = label_width;
      break;
    case JUST_CENTER:
      dx = 0.0;
      break;
    case JUST_RIGHT:
      dx = - label_width;
      break;
    }

  translation_x = costheta * dx;
  translation_y = sintheta * dx;

  /* vector along baseline of label, and an orthogonal vector which is the
     other side of a rectangle containing the portion of the string above
     the baseline (both in the user frame) */

  horizontal_x = costheta * label_width;
  horizontal_y = sintheta * label_width;

  vertical_x =  - sintheta * label_ascent;
  vertical_y =    costheta * label_ascent;
  
  /* Convert two above orthogonal vectors to the device frame, and compute
     their lengths.  In the device frame they may no longer be orthogonal.
     But xfig supports setting up only rectangular `hot regions', so we'll
     use their lengths as the sides of a rectangle.  If user coor->device
     coor map is highly sheared, this would be inappropriate. 

     Incidentally, the height of the rectangular hot region should really
     be the string ascent (from its bounding box), not the font ascent.
     But since we don't include the bounding boxes of individual characters
     in our g_fontdb.c, we have no way of computing the former. */

  horizontal_fig_x = XDV(horizontal_x, horizontal_y);
  horizontal_fig_y = YDV(horizontal_x, horizontal_y);  
  horizontal_fig_length = sqrt(horizontal_fig_x * horizontal_fig_x
				+ horizontal_fig_y * horizontal_fig_y);

  /* text angle in device frame (note flipped-y convention) */
  angle_device = - _xatan2 (horizontal_fig_y, horizontal_fig_x);
  if (angle_device == 0.0)
    angle_device = 0.0;		/* remove sign bit if any */
  
  vertical_fig_x = XDV(vertical_x, vertical_y);
  vertical_fig_y = YDV(vertical_x, vertical_y);  
  vertical_fig_length = sqrt(vertical_fig_x * vertical_fig_x
				+ vertical_fig_y * vertical_fig_y);
  
  /* where we should start from, in device frame (i.e. in Fig units) */
  initial_x = XD((_plotter->drawstate->pos).x, (_plotter->drawstate->pos).y);
  initial_y = YD((_plotter->drawstate->pos).x, (_plotter->drawstate->pos).y);

  /* evaluate fig colors lazily, i.e. only when needed */
  _plotter->set_pen_color();
  
  /* escape all backslashes in the text string, before output */
  len = strlen ((char *)s);
  ptr = (unsigned char *)_plot_xmalloc ((unsigned int)(4 * len + 1));
  t = ptr;
  while (*s)
    {
      switch (*s)
	{
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

  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  sprintf(_plotter->page->point,
	  "#TEXT\n%d %d %d %d %d %d %.3f %.3f %d %.3f %.3f %d %d %s\\001\n",
	  4,			/* text object */
	  /* xfig supports 3 justification types: left, center, or right. */
	  h_just,		/* horizontal justification type */
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _ps_font_info[master_font_index].fig_id, /* Fig font id */
	  (double)_plotter->drawstate->fig_font_point_size, /* point size (float) */
	  angle_device,		/* text rotation in radians (float) */
	  FONT_TYPE_PS,		/* Fig font type */
	  /* these next two are used only for setting up `hot spots' */
	  vertical_fig_length, /* string height, Fig units (float) */
	  horizontal_fig_length, /* string width, Fig units (float) */
	  /* coors of origin of label, in Fig units */
	  IROUND(initial_x), 
	  IROUND(initial_y),
	  t);			/* munged string */
  free (t);
  _update_buffer (_plotter->page);

  /* reposition after printing substring */
  _plotter->drawstate->pos.x += translation_x;
  _plotter->drawstate->pos.y += translation_y;
 
  return label_width;
}
