/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

#include "sys-defines.h"
#include "extern.h"

static const double _identity_matrix[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

/* forward references */
static void _write_svg_point_style ____P((plOutbuf *page, const plDrawState *drawstate));

void
#ifdef _HAVE_PROTOS
_s_paint_point (S___(Plotter *_plotter))
#else
_s_paint_point (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  sprintf (_plotter->data->page->point, "<circle ");
  _update_buffer (_plotter->data->page);

  _s_set_matrix (R___(_plotter) 
		 _plotter->drawstate->transform.m_user_to_ndc,
		 _identity_matrix); 

  sprintf (_plotter->data->page->point,
	   "cx=\"%.5g\" cy=\"%.5g\" r=\"%s\" ",
	   _plotter->drawstate->pos.x,
	   _plotter->drawstate->pos.y,
	   "0.5px");		/* diameter = 1 pixel */
  _update_buffer (_plotter->data->page);
  
  _write_svg_point_style (_plotter->data->page, _plotter->drawstate);

  sprintf (_plotter->data->page->point,
	   "/>\n");
  _update_buffer (_plotter->data->page);

  return;
}

static void
#ifdef _HAVE_PROTOS
_write_svg_point_style (plOutbuf *page, const plDrawState *drawstate)
#else
_write_svg_point_style (page, drawstate)
     plOutbuf *page; 
     const plDrawState *drawstate; 
#endif
{
  char color_buf[8];		/* enough room for "#ffffff", incl. NUL */

  sprintf (page->point, "style=\"");
  _update_buffer (page);
  
  sprintf (page->point, "stroke:none;");
  _update_buffer (page);
  
  sprintf (page->point, "fill:%s;",
	   _libplot_color_to_svg_color (drawstate->fgcolor, color_buf));
  _update_buffer (page);

  sprintf (page->point, "\"");
  _update_buffer (page);
}
