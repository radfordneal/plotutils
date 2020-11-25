/* This file contains the savestate method, which is a GNU extension to
   libplot.  It creates a new drawing state and pushes it onto the stack of
   drawing states.  By definition, a `drawing state' comprises the set of
   drawing attributes, and the state of any path being incrementally drawn.

   The new state will have the same drawing attributes as the old state.
   If a path was being drawn incrementally in the old state, the new state
   will not contain it.  The old state may be returned to by invoking the
   restorestate routine, which pops drawing states off the stack.  If the
   incremental drawing of a path was in progress, it may be returned to at
   that time.

   This version of savestate() assumes that the device-specific part of the
   drawing state contains no strings.  Plotter objects for which this is
   not true must not use this version, since they need to call malloc() to
   allocate space for the string in the new state. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_savestate(void)
#else
_g_savestate()
#endif
{
  pl_DrawState *oldstate = _plotter->drawstate;
  const pl_DrawState *copyfrom;

  if (!_plotter->open)
    {
      _plotter->error ("savestate: invalid operation");
      return -1;
    }

  /* create a new state */
  _plotter->drawstate = (pl_DrawState *)_plot_xmalloc (sizeof(pl_DrawState));
  
  /* copy from old state unless there wasn't any (i.e. an empty stack), in
     which case copy from the default drawing state (see g_defstate.c) */
  copyfrom = oldstate ? oldstate : &_default_drawstate;
  memcpy (_plotter->drawstate, copyfrom, sizeof(pl_DrawState));

  /* elements of state that are strings are treated specially */
  _plotter->drawstate->fill_rule = (char *)_plot_xmalloc (strlen (copyfrom->fill_rule) + 1);
  _plotter->drawstate->line_mode = (char *)_plot_xmalloc (strlen (copyfrom->line_mode) + 1);
  _plotter->drawstate->join_mode = (char *)_plot_xmalloc (strlen (copyfrom->join_mode) + 1);
  _plotter->drawstate->cap_mode = (char *)_plot_xmalloc (strlen (copyfrom->cap_mode) + 1);
  strcpy (_plotter->drawstate->fill_rule, copyfrom->fill_rule);
  strcpy (_plotter->drawstate->line_mode, copyfrom->line_mode);
  strcpy (_plotter->drawstate->join_mode, copyfrom->join_mode);
  strcpy (_plotter->drawstate->cap_mode, copyfrom->cap_mode);

  /* dash array, if non-empty, is treated specially too */
  if (copyfrom->dash_array_len > 0)
    {
      int i;

      _plotter->drawstate->dash_array = (double *)_plot_xmalloc (copyfrom->dash_array_len * sizeof(double));
      for (i = 0; i < copyfrom->dash_array_len; i++)
	_plotter->drawstate->dash_array[i] = copyfrom->dash_array[i];
    }

  /* The font_name, font_type, typeface_index, and font_index fields are
     special, since for the initial drawing state they're Plotter-dependent.

     For later drawing states, we just copy them from the previous state.
     Since only the first (font_name) is a string, for later states we
     don't worry about the other three: they've already been copied.

     The fill_type field is also treated specially in the initial drawing
     state, because not all Plotters support both types of filling (odd
     vs. nonzero winding number). */

  if (oldstate)			/* a later state */
    {
      _plotter->drawstate->font_name = (char *)_plot_xmalloc (strlen (copyfrom->font_name) + 1);  
      strcpy (_plotter->drawstate->font_name, copyfrom->font_name);  
    }
  else				/* initial state, i.e. first on stack */
    {
      char *font_name_init;
      int typeface_index_init, font_index_init;
      
      switch (_plotter->default_font_type)
	{
	case F_HERSHEY:
	default:
	  font_name_init = DEFAULT_HERSHEY_FONT;
	  typeface_index_init = DEFAULT_HERSHEY_TYPEFACE_INDEX;
	  font_index_init = DEFAULT_HERSHEY_FONT_INDEX;	  
	  break;
	case F_POSTSCRIPT:
	  font_name_init = DEFAULT_POSTSCRIPT_FONT;
	  typeface_index_init = DEFAULT_POSTSCRIPT_TYPEFACE_INDEX;
	  font_index_init = DEFAULT_POSTSCRIPT_FONT_INDEX;	  
	  break;
	case F_PCL:
	  font_name_init = DEFAULT_PCL_FONT;
	  typeface_index_init = DEFAULT_PCL_TYPEFACE_INDEX;
	  font_index_init = DEFAULT_PCL_FONT_INDEX;	  
	  break;
	case F_STICK:
	  font_name_init = DEFAULT_STICK_FONT;
	  typeface_index_init = DEFAULT_STICK_TYPEFACE_INDEX;
	  font_index_init = DEFAULT_STICK_FONT_INDEX;	  
	  break;
	}

      _plotter->drawstate->font_name = (char *)_plot_xmalloc (strlen (font_name_init) + 1);  
      strcpy (_plotter->drawstate->font_name, font_name_init);  
      _plotter->drawstate->font_type = _plotter->default_font_type;      
      _plotter->drawstate->typeface_index = typeface_index_init;
      _plotter->drawstate->font_index = font_index_init;      

      /* Examine default fill mode.  If Plotter doesn't support it, use the
	 other fill mode. */
      if (_plotter->drawstate->fill_rule_type == FILL_ODD_WINDING
	  && _plotter->have_odd_winding_fill == 0)
	_plotter->drawstate->fill_rule_type = FILL_NONZERO_WINDING;
      else if (_plotter->drawstate->fill_rule_type == FILL_NONZERO_WINDING
	  && _plotter->have_nonzero_winding_fill == 0)
	_plotter->drawstate->fill_rule_type = FILL_ODD_WINDING;
    }

  /* Our memcpy copied the path under constrution (if there is one).  So we
     knock it out, to start afresh */
  _plotter->drawstate->datapoints = NULL;
  _plotter->drawstate->points_in_path = 0;
  _plotter->drawstate->datapoints_len = 0;

  /* install new state at head of the state list */
  _plotter->drawstate->previous = oldstate;

  return 0;
}
