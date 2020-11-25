/* This file contains the savestate routine, which is a GNU extension to
   libplot.  It creates a new drawing state and pushes it onto the stack of
   drawing states.  By definition, a `drawing state' comprises the set of
   drawing attributes, and the state of any polyline being incrementally
   drawn.

   The new state will have the same drawing attributes as the old state.
   If a polyline was being drawn incrementally in the old state, the new
   state will not contain it.  The old state may be returned to by calling
   the restorestate routine, which pops drawing states off the stack.
   If the incremental drawing of a polyline was in progress, it may be
   returned to at that time. */

/* For libplotX, we construct a graphics context for the new drawing
   state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "defaultstate.h"	/* contains initialization of new state */

/* pointer to state at head of linked list */
State *_drawstate = NULL;	/* initially, list is empty */

int
savestate()
{
  State *oldstate = _drawstate;
  State *copyfrom;
  unsigned long gcmask;
  XGCValues gcv;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: savestate() called when graphics device not open\n");
      return -1;
    }

  /* create a new state */
  _drawstate = (State *)_plot_xmalloc (sizeof(State));
  
  /* copy from old state unless there wasn't any (i.e. an empty stack) */
  copyfrom = oldstate ? oldstate : &_default_drawstate;
  memcpy (_drawstate, copyfrom, sizeof(State));

  /* elements of state that are strings are treated specially */
  _drawstate->line_mode = (char *)_plot_xmalloc (strlen (copyfrom->line_mode) + 1);
  _drawstate->join_mode = (char *)_plot_xmalloc (strlen (copyfrom->join_mode) + 1);
  _drawstate->cap_mode = (char *)_plot_xmalloc (strlen (copyfrom->cap_mode) + 1);
  _drawstate->font_name = (char *)_plot_xmalloc (strlen (copyfrom->font_name) + 1);  
  strcpy (_drawstate->line_mode, copyfrom->line_mode);
  strcpy (_drawstate->join_mode, copyfrom->join_mode);
  strcpy (_drawstate->cap_mode, copyfrom->cap_mode);
  strcpy (_drawstate->font_name, copyfrom->font_name);  

  /* Our memcpy copied the polyline object (if there is one), as well as
     the drawing attributes.  So we knock it out, to start afresh */

  _drawstate->datapoints = NULL;
  _drawstate->PointsInLine = 0;
  _drawstate->datapoints_len = 0;

  /* install new state at head of the state list */
  _drawstate->previous = oldstate;

  /* now prepare new GC, focusing on attributes we use */
  if (_drawstate->previous) /* can copy contents of old GC */
    {
      gcmask = GCForeground | GCBackground 
	| GCPlaneMask | GCFunction | GCArcMode
	|  GCLineStyle | GCLineWidth | GCJoinStyle | GCCapStyle | GCFont;
      XGetGCValues (_xdata.dpy, _drawstate->previous->gc, gcmask, &gcv);
      _drawstate->gc = XCreateGC (_xdata.dpy, _xdata.pixmap, gcmask, &gcv);
    }
  else
    /* initializing, must build new GC from scratch */

    /* NOTE: the quantities CROCK_DEFAULT_LINE_WIDTH_IN_PIXELS and
       CROCK_DEFAULT_FONT_SIZE_IN_PIXELS are defined in extern.h.  They
       have bad names because they really should be taken from
       _default_drawstruct, like the other state variables in the GC,
       except that they can't be, because space() hasn't been called yet,
       and those variables haven't been set.  Aargh. */
    {
      gcmask = GCForeground | GCBackground
	| GCPlaneMask | GCFunction | GCArcMode;
      gcv.background = _default_drawstate.x_bgcolor; /* set in openpl.c */
      gcv.foreground = _default_drawstate.x_fgcolor; /* set in openpl.c */
      gcv.plane_mask = AllPlanes;
      gcv.function = GXcopy;
      gcv.arc_mode = ArcChord;	/* libplot convention */
      _drawstate->gc = XCreateGC (_xdata.dpy, _xdata.pixmap, gcmask, &gcv);

      /* make the just-created GC take on default values */
      linemod (_default_drawstate.line_mode);
      flinewidth ((double)CROCK_DEFAULT_LINE_WIDTH_IN_PIXELS);
      joinmod (_default_drawstate.join_mode);
      capmod (_default_drawstate.cap_mode);
      color (_default_drawstate.fgcolor.red,
	     _default_drawstate.fgcolor.green,
	     _default_drawstate.fgcolor.blue);
      fillcolor (_default_drawstate.fillcolor.red,
		 _default_drawstate.fillcolor.green,
		 _default_drawstate.fillcolor.blue);

      /* state is first on stack, so retrieve default font from X server */
      if (!_retrieve_font_internal(_default_drawstate.font_name, 
				   (double)CROCK_DEFAULT_FONT_SIZE_IN_PIXELS,
				   _default_drawstate.text_rotation))
	fprintf (stderr, "libplot: couldn't initialize font to %s at size %f and rotation angle %f\n", 
		 _default_drawstate.font_name, 
		 _default_drawstate.font_size, 
		 _default_drawstate.text_rotation);
    }
  _handle_x_events();

  return 0;
}
