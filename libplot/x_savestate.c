/* This file contains the savestate method, which is a GNU extension to
   libplot.  It creates a new drawing state and pushes it onto the stack of
   drawing states.  By definition, a `drawing state' comprises the set of
   drawing attributes, and the state of any path being incrementally drawn.

   The new state will have the same drawing attributes as the old state.
   If a path was being drawn incrementally in the old state, the new state
   will not contain it.  The old state may be returned to by invoking the
   restorestate method, which pops drawing states off the stack.  If the
   incremental drawing of a path was in progress, it may be returned to at
   that time. */

/* This version is for XDrawablePlotters (and XPlotters).  It constructs
   X11 GC's (i.e. graphics contexts) for the new drawing state.  We copy
   into them, from each of the current GC's, all the attributes we're
   interested in. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_savestate(S___(Plotter *_plotter))
#else
_x_savestate(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  Drawable drawable;
      
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "savestate: invalid operation");
      return -1;
    }

  /* invoke generic method: push new drawing state onto stack, a copy
     of the current one, or of the default one if stack is empty */
  _g_savestate (S___(_plotter));

  /* Must now create the X-specified drawing state elements that are
     pointers (e.g., GC's or lists). */

  /* determine which if either drawable we'll construct the GC's for */
  if (_plotter->x_drawable1)
    drawable = _plotter->x_drawable1;
  else if (_plotter->x_drawable2)
    drawable = _plotter->x_drawable2;
  else
    drawable = (Drawable)NULL;

  if (drawable != (Drawable)NULL)
    /* prepare GC's for new drawing state, by copying attributes we use */
    {
      unsigned long gcmask_fg, gcmask_fill, gcmask_bg;

      gcmask_fg = 
	/* constant attributes (never altered) */
	GCPlaneMask | GCFunction
	/* drawing attributes set by _x_set_attributes() */
	/* NOTE: we also use GCDashOffset and GCDashList, but Xlib does not
	   support retrieving the dash list from a GC, so we'll copy the
	   dashing style in another (painful) way */
	| GCLineStyle | GCLineWidth | GCJoinStyle | GCCapStyle
	/* other GC elements set by the X Drawable driver */
	| GCForeground | GCFont;
      
      gcmask_fill = 
	/* constant attributes (never altered) */
	GCPlaneMask | GCFunction | GCArcMode 
	/* filling attributes set by _x_set_attributes() */
	| GCFillRule
	/* other GC elements set by the X Drawable driver */
	| GCForeground;
      
      gcmask_bg = 
	/* constant attributes (never altered) */
	GCPlaneMask | GCFunction 
	/* other GC elements set by the X Drawable driver */
	| GCForeground;
      
      if (_plotter->drawstate->previous)
	/* there was a previous drawing state, so copy contents of old GC's */
	{
	  XGCValues gcv;

	  /* copy GC used for drawing */
	  XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_fg, 
			gcmask_fg, &gcv);
	  _plotter->drawstate->x_gc_fg = XCreateGC (_plotter->x_dpy, drawable, 
						    gcmask_fg, &gcv);
	  if (gcv.line_style != LineSolid)
	    /* copy dash style info from previous state */
	    {
	      int i, dash_list_len;
	      char *dash_list;

	      /* add dash style elements to GC used for drawing */
	      XSetDashes (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
			  _plotter->drawstate->previous->x_gc_dash_offset, 
			  _plotter->drawstate->previous->x_gc_dash_list,
			  _plotter->drawstate->previous->x_gc_dash_list_len);

	      /* add non-opaque dash style elements */
	      dash_list_len = 
		_plotter->drawstate->previous->x_gc_dash_list_len;
	      dash_list = (char *)_plot_xmalloc (dash_list_len * sizeof(char));
	      for (i = 0; i < dash_list_len; i++)
		dash_list[i] =
		  _plotter->drawstate->previous->x_gc_dash_list[i];
	      _plotter->drawstate->x_gc_dash_list = dash_list;

	      /* these two were already added by the copy operation that
		 took place in _g_savestate(), but we'll add them again */
	      _plotter->drawstate->x_gc_dash_list_len = dash_list_len;
	      _plotter->drawstate->x_gc_dash_offset = 
		_plotter->drawstate->previous->x_gc_dash_offset;
	    }
	  else
	    {
	      _plotter->drawstate->x_gc_dash_list = (char *)NULL;
	      _plotter->drawstate->x_gc_dash_list_len = 0;
	      _plotter->drawstate->x_gc_dash_offset = 0;
	    }

	  /* copy GC used for filling */
	  XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_fill, 
			gcmask_fill, &gcv);
	  _plotter->drawstate->x_gc_fill = XCreateGC (_plotter->x_dpy, drawable, 
						      gcmask_fill, &gcv);
	  /* copy GC used for erasing */
	  XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_bg, 
			gcmask_bg, &gcv);
	  _plotter->drawstate->x_gc_bg = XCreateGC (_plotter->x_dpy, drawable, 
						    gcmask_bg, &gcv);
	}
      else
	/* Drawing state stack is empty, so we're just starting out: this
           function must have been invoked as part of openpl().  So build
           new GC's from scratch */
	{
	  XGCValues gcv_fg, gcv_fill, gcv_bg;

	  /* Initialize GC used for drawing.  (Always initialize the line
	     style to LineSolid, irrespective of what the default drawing
	     state contains; it would be silly for the default drawing
	     state to include a non-solid value for the line style.) */
	  gcv_fg.plane_mask = AllPlanes;
	  gcv_fg.function = GXcopy;
	  gcv_fg.line_width = _default_drawstate.x_gc_line_width;
	  gcv_fg.line_style = LineSolid;
	  gcv_fg.join_style = _default_drawstate.x_gc_join_style;
	  gcv_fg.cap_style = _default_drawstate.x_gc_cap_style;
	  gcmask_fg &= ~(GCFont); /* initialized much later; see below */
	  gcmask_fg &= ~(GCForeground);	/* color is initialized separately */

	  /* initialize GC used for filling */
	  gcv_fill.plane_mask = AllPlanes;
	  gcv_fill.function = GXcopy;
	  gcv_fill.arc_mode = ArcChord; /* libplot convention */
	  gcv_fill.fill_rule = _default_drawstate.x_gc_fill_rule;
	  gcmask_fill &= ~(GCForeground); /* color is initialized separately */

	  /* initialize GC used for erasing */
	  gcv_bg.plane_mask = AllPlanes;
	  gcv_bg.function = GXcopy;
	  gcmask_bg &= ~(GCForeground); /* color is initialized separately */

	  /* create the 3 GC's */
	  _plotter->drawstate->x_gc_fg = 
	    XCreateGC (_plotter->x_dpy, drawable, gcmask_fg, &gcv_fg);
	  _plotter->drawstate->x_gc_fill = 
	    XCreateGC (_plotter->x_dpy, drawable, gcmask_fill, &gcv_fill);
	  _plotter->drawstate->x_gc_bg = 
	    XCreateGC (_plotter->x_dpy, drawable, gcmask_bg, &gcv_bg);

	  /* set X-specific elements in the drawing state, specifying
	     (non-opaquely) what the 3 GC's contain */
	  _plotter->drawstate->x_gc_line_width = gcv_fg.line_width;
	  _plotter->drawstate->x_gc_line_style = gcv_fg.line_style;
	  _plotter->drawstate->x_gc_join_style = gcv_fg.join_style;
	  _plotter->drawstate->x_gc_cap_style = gcv_fg.cap_style;
	  _plotter->drawstate->x_gc_dash_list = (char *)NULL;
	  _plotter->drawstate->x_gc_dash_list_len = 0;
	  _plotter->drawstate->x_gc_dash_offset = 0;
	  _plotter->drawstate->x_gc_fill_rule = gcv_fill.fill_rule;

	  /* do the separate initialization of color (i.e. GCForeground
             element) in each GC */
	  _plotter->set_pen_color (S___(_plotter));
	  _plotter->set_fill_color (S___(_plotter));
	  _plotter->set_bg_color (S___(_plotter));

	  /* At this point, all 3 GC's are functional, except the GC used
	     for drawing lacks a GCFont element.

	     We do not retrieve a font from the X server here; not even a
	     default font.  space(), when invoked (which we require after
	     each invocation of openpl()), will select a default size for
	     the font.  A font will be retrieved from the X server only
	     when fontname/fontsize/textangle is invoked to select a
	     different font, or when alabel/labelwidth is invoked (see
	     g_alabel.c).

	     The invocation of space() will also set the line width in the
	     drawing state.  Any changed attributes, such as line width,
	     will be written to the GC's just before drawing; see
	     g_attribs.c. */
	}
    }
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return 0;
}
