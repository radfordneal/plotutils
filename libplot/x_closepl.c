#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_x_end_page (S___(Plotter *_plotter))
#else
_x_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* Xdrawable Plotters support double buffering `by hand', so check for it */

  if (_plotter->x_double_buffering == DBL_BY_HAND)
    /* copy final frame of buffered graphics from pixmap serving as
       graphics buffer, to window */
    {
      /* compute rectangle size; note flipped-y convention */
      int window_width = (_plotter->data->imax - _plotter->data->imin) + 1;
      int window_height = (_plotter->data->jmin - _plotter->data->jmax) + 1;
      
      if (_plotter->x_drawable1)
	XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable1,
		   _plotter->drawstate->x_gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      if (_plotter->x_drawable2)
	XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable2,
		   _plotter->drawstate->x_gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      
      /* no more need for pixmap, so free it (if there is one) */
      if (_plotter->x_drawable1 || _plotter->x_drawable2)
	XFreePixmap (_plotter->x_dpy, _plotter->x_drawable3);
    }

  /* do teardown of X-specific elements of the first drawing state on the
     drawing state stack */
  _x_delete_gcs_from_first_drawing_state (S___(_plotter));
  
  return true;
}

void
#ifdef _HAVE_PROTOS
_x_delete_gcs_from_first_drawing_state (S___(Plotter *_plotter))
#else
_x_delete_gcs_from_first_drawing_state (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* free graphics contexts, if we have them -- and to have them, must have
     at least one drawable (see x_savestate.c) */
  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_bg);
    }
}
