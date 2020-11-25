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
   X GC's (i.e. graphics contexts) for the new drawing state. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_x_savestate(void)
#else
_x_savestate()
#endif
{
  unsigned long gcmask;
  XGCValues gcv;
  Drawable drawable;
      
  if (!_plotter->open)
    {
      _plotter->error ("savestate: invalid operation");
      return -1;
    }

  /* invoke generic method: push new drawing state onto stack, a copy
     of the current one, or of the default one if stack is empty */
  _g_savestate();

  /* determine which if either drawable we'll construct the GC's for */
  if (_plotter->x_drawable1)
    drawable = _plotter->x_drawable1;
  else if (_plotter->x_drawable2)
    drawable = _plotter->x_drawable2;
  else
    drawable = (Drawable)NULL;

  if (drawable != (Drawable)NULL)
    /* prepare GC's for new drawing state, focusing on attributes we use */
    {
      if (_plotter->drawstate->previous)
	/* there was a previous drawing state, can copy contents of old GC's */
	{
	  gcmask = GCForeground | GCBackground 
	    | GCPlaneMask | GCFunction | GCArcMode | GCFillRule
	      |  GCLineStyle | GCLineWidth | GCJoinStyle | GCCapStyle | GCFont;
	  /* copy GC used for drawing */
	  XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_fg, 
			gcmask, &gcv);
	  _plotter->drawstate->x_gc_fg = XCreateGC (_plotter->x_dpy, drawable, 
						  gcmask, &gcv);
	  /* copy GC used for filling */
	  XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_fill, 
			gcmask, &gcv);
	  _plotter->drawstate->x_gc_fill = XCreateGC (_plotter->x_dpy, drawable, 
						    gcmask, &gcv);
	  /* copy GC used for erasing */
	  XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_bg, 
			gcmask, &gcv);
	  _plotter->drawstate->x_gc_bg = XCreateGC (_plotter->x_dpy, drawable, 
						  gcmask, &gcv);
	}
      else
	/* stack must have been empty, must build new GC's from scratch */
	{
	  gcmask = GCPlaneMask | GCFunction | GCArcMode | GCFillRule;
	  gcv.plane_mask = AllPlanes;
	  gcv.function = GXcopy;
	  gcv.arc_mode = ArcChord; /* libplot convention */
	  switch (_default_drawstate.fill_rule_type)
	    {
	    case FILL_ODD_WINDING:
	    default:
	      gcv.fill_rule = EvenOddRule;
	      break;
	    case FILL_NONZERO_WINDING:
	      gcv.fill_rule = WindingRule;
	      break;
	    }

	  _plotter->drawstate->x_gc_fg = 
	    XCreateGC (_plotter->x_dpy, drawable, gcmask, &gcv);
	  _plotter->drawstate->x_gc_fill = 
	    XCreateGC (_plotter->x_dpy, drawable, gcmask, &gcv);
	  _plotter->drawstate->x_gc_bg = 
	    XCreateGC (_plotter->x_dpy, drawable, gcmask, &gcv);

	  /* set the foreground color in each of our three GC's */
	  /* UNNECESSARY? */
	  _plotter->set_pen_color ();
	  _plotter->set_fill_color ();
	  _plotter->set_bg_color ();	  

	  /* We do not retrieve a font from the X server here; not even a
	     default font.  space(), when invoked (which we require after
	     each invocation of openpl()), will select a default size for
	     the font.  A font will be retrieved from the X server only
	     when fontname/fontsize/textangle is invoked to select a
	     different font, or when alabel/labelwidth is invoked (see
	     g_alabel.c). */
	}
    }
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events();

  return 0;
}
