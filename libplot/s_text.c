/* This prints a single-font, single-font-size label.  When this is called,
   the current point is on the intended baseline of the label.  */

/* This version is for SVGPlotters.  We use the ISO-Latin-1 encoding, so <
   > & and left-quote and double-quote are the only characters we need to
   escape. */

#include "sys-defines.h"
#include "extern.h"

/* maximum length we support */
#define MAX_SVG_STRING_LEN 256

/* forward references */
static void _write_svg_text_style ____P((plOutbuf *page, const plDrawState *drawstate, int h_just, int v_just));

typedef struct
{
  char c;
  const char *s;
}
plCharEscape;

#define NUM_SVG_CHAR_ESCAPES 5
static const plCharEscape _svg_char_escapes[NUM_SVG_CHAR_ESCAPES] =
{
  {'\'', "apos"},
  {'"', "quot"},
  {'&', "amp"},
  {'<', "lt"},
  {'>', "gt"}
};
#define MAX_SVG_CHAR_ESCAPE_LEN 4 /* i.e., length of "apos" or "quot" */

/* SVG horizontal alignment styles, i.e., text-anchor attribute, indexed by
   internal number (left/center/right) */
static const char * _svg_horizontal_alignment_style[] =
{ "start", "middle", "end" };

/* SVG vertical alignment styles, i.e., baseline-identifier attribute,
   indexed by internal number (top/half/base/bottom/cap) */
static const char * _svg_vertical_alignment_style[] =
{ "text-top", "centerline", "baseline", "text-bottom", "topline" };

double
#ifdef _HAVE_PROTOS
_s_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
#else
_s_paint_text_string (R___(_plotter) s, h_just, v_just)
     S___(Plotter *_plotter;)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
     int v_just;  /* vertical justification: JUST_TOP, HALF, BASE, BOTTOM */
#endif
{
  const unsigned char *sp = s;
  unsigned char *t, *tp;
  int n = 0;
  double local_matrix[6];
  double angle = _plotter->drawstate->text_rotation;
  
  /* replace certain printable ASCII characters by entities */
  tp = t = (unsigned char *)_plot_xmalloc ((2 + MAX_SVG_CHAR_ESCAPE_LEN) * strlen ((const char *)s) + 1);
  while (*sp && n < MAX_SVG_STRING_LEN)
    {
      bool matched;
      int i;
      
      matched = false;
      for (i = 0; i < NUM_SVG_CHAR_ESCAPES; i++)
	{
	  if (*sp == (unsigned char)_svg_char_escapes[i].c)
	    {
	      matched = true;
	      break;
	    }
	}
      if (matched)
	{
	  *tp++ = (unsigned char)'&';
	  strcpy ((char *)tp, _svg_char_escapes[i].s);
	  tp += strlen (_svg_char_escapes[i].s);
	  *tp++ = (unsigned char)';';
	}
      else
	*tp++ = *sp;

      sp++;
      n++;
    }
  *tp = '\0';
  
  sprintf (_plotter->data->page->point, "<text ");
  _update_buffer (_plotter->data->page);

  /* CTM equals CTM_local * CTM_base, if matrix multiplication is defined
     as in PS and libplot. (Which is the opposite of the SVG convention,
     since SVG documentation uses column vectors instead of row vectors, so
     that the CTM is effectively transposed.  Although SVG's matrix()
     construct uses Ps order for the six matrix elements... go figure.)

     Here CTM_local rotates by the libplot's text angle parameter, and
     translates to the correct position.  And CTM_base is libplot's current
     user_to_ndc transformation matrix.  We separate them because we use
     the CTM of the first-plotted object on the page as the page's global
     transformation matrix, and if that object happens to be a text object,
     we'd like it to simply to be the current user_to_ndc transformation
     matrix, i.e. not to include irrelevancies such as the text position
     and angle.

     Sigh... If only things were so simple.  SVG's native coordinate frame,
     which libplot's user coordinates must ultimately be mapped to,
     unfortunately uses a flipped-y convention, unlike PS and libplot.
     (The global flipping of y, relative to libplot's NDC coordinates, is
     accomplished by a scale(1,-1) that's placed at the head of the SVG
     file; see s_output.c.)  This flipping has a special effect on the
     drawing of text strings, though no other libplot primitive.  For
     everything to work out when drawing a text string, we must precede the
     sequence of transformations leading from user coordinates to native
     SVG coordinates by an initial scale(1,-1).  CTM_local, as defined
     above, must have two elements sign-flipped (see below).  Trust me. */

  local_matrix[0] = cos (M_PI * angle / 180.0);
  local_matrix[1] = sin (M_PI * angle / 180.0);
  local_matrix[2] = -sin (M_PI * angle / 180.0) * (-1);	/* SEE ABOVE */
  local_matrix[3] = cos (M_PI * angle / 180.0) * (-1); /* SEE ABOVE */
  local_matrix[4] = _plotter->drawstate->pos.x;
  local_matrix[5] = _plotter->drawstate->pos.y;
  _s_set_matrix (R___(_plotter) 
		 _plotter->drawstate->transform.m_user_to_ndc, 
		 local_matrix); 

  _write_svg_text_style (_plotter->data->page, _plotter->drawstate, 
			 h_just, v_just);

  sprintf (_plotter->data->page->point, ">");
  _update_buffer (_plotter->data->page);
  
  sprintf (_plotter->data->page->point, "%s",
	   (char *)t);
  _update_buffer (_plotter->data->page);
  
  sprintf (_plotter->data->page->point, "</text>\n");
  _update_buffer (_plotter->data->page);

  free (t);

  return _plotter->get_text_width (R___(_plotter) s);
}

static void
#ifdef _HAVE_PROTOS
_write_svg_text_style (plOutbuf *page, const plDrawState *drawstate, int h_just, int v_just)
#else
_write_svg_text_style (page, drawstate, h_just, v_just)
     plOutbuf *page; 
     const plDrawState *drawstate; 
     int h_just, v_just;
#endif
{
  const char *ps_name, *css_family, *css_generic_family; /* last may be NULL */
  const char *css_style, *css_weight, *css_stretch;
  bool css_family_is_ps_name;
  char color_buf[8];		/* enough room for "#ffffff", incl. NUL */

  /* extract official PS font name, and CSS font specification, from master
     table of PS [or PCL] fonts, in g_fontdb.c */
  switch (drawstate->font_type)
    {
      int master_font_index;

    case F_POSTSCRIPT:
      master_font_index =
	(_ps_typeface_info[drawstate->typeface_index].fonts)[drawstate->font_index];
      ps_name = _ps_font_info[master_font_index].ps_name;
      css_family = _ps_font_info[master_font_index].css_family;
      css_generic_family = _ps_font_info[master_font_index].css_generic_family;
      css_style = _ps_font_info[master_font_index].css_style;
      css_weight = _ps_font_info[master_font_index].css_weight;
      css_stretch = _ps_font_info[master_font_index].css_stretch;
      break;
    case F_PCL:
      master_font_index =
	(_pcl_typeface_info[drawstate->typeface_index].fonts)[drawstate->font_index];
      ps_name = _pcl_font_info[master_font_index].ps_name;
      css_family = _pcl_font_info[master_font_index].css_family;
      css_generic_family = _pcl_font_info[master_font_index].css_generic_family;
      css_style = _pcl_font_info[master_font_index].css_style;
      css_weight = _pcl_font_info[master_font_index].css_weight;
      css_stretch = _pcl_font_info[master_font_index].css_stretch;
      break;
    default:			/* shouldn't happen */
      return;
      break;
    }

  sprintf (page->point, "style=\"");
  _update_buffer (page);

  if (strcmp (ps_name, css_family) == 0)
    /* no need to specify both */
    css_family_is_ps_name = true;
  else
    css_family_is_ps_name = false;

  if (css_generic_family)
    {
      if (css_family_is_ps_name)
	sprintf (page->point, "font-family:'%s',%s;",
		 css_family, css_generic_family);
      else
	sprintf (page->point, "font-family:%s,'%s',%s;",
		 ps_name, css_family, css_generic_family);
    }
  else
    {
      if (css_family_is_ps_name)
	sprintf (page->point, "font-family:'%s';",
		 css_family);
      else
	sprintf (page->point, "font-family:%s,'%s';",
		 ps_name, css_family);
    }
  _update_buffer (page);
  
  if (strcmp (css_style, "normal") != 0) /* not default */
    {
      sprintf (page->point, "font-style:%s;",
	       css_style);
      _update_buffer (page);
    }

  if (strcmp (css_weight, "normal") != 0) /* not default */
    {
      sprintf (page->point, "font-weight:%s;",
	       css_weight);
      _update_buffer (page);
    }

  if (strcmp (css_stretch, "normal") != 0) /* not default */
    {
      sprintf (page->point, "font-stretch:%s;",
	       css_stretch);
      _update_buffer (page);
    }

  sprintf (page->point, "font-size:%.5g;",
	   drawstate->font_size);
  _update_buffer (page);

  if (h_just != JUST_LEFT)	/* not default */
    {
      sprintf (page->point, "text-anchor:%s;",
	       _svg_horizontal_alignment_style[h_just]);
      _update_buffer (page);
    }

  if (v_just != JUST_BASE)	/* not default */
    {
      sprintf (page->point, "baseline-identifier:%s;",
	       _svg_vertical_alignment_style[v_just]);
      _update_buffer (page);
    }

  /* currently, we never draw outlines; we only fill */
  sprintf (page->point, "stroke:none;");
  _update_buffer (page);

  if (drawstate->pen_type)
    /* according to libplot convention, text should be filled, and since
       SVG's default filling is "none", we must say so */
    {
      sprintf (page->point, "fill:%s;",
	       _libplot_color_to_svg_color (drawstate->fgcolor, color_buf));
      _update_buffer (page);
    }
  
  sprintf (page->point, "\"");
  _update_buffer (page);
}
