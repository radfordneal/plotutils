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
_g_savestate(S___(Plotter *_plotter))
#else
_g_savestate(S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  plDrawState *oldstate = _plotter->drawstate;
  const plDrawState *copyfrom;
  char *fill_rule, *line_mode, *join_mode, *cap_mode;
  char *font_name;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "savestate: invalid operation");
      return -1;
    }

  /* create a new state */
  _plotter->drawstate = (plDrawState *)_plot_xmalloc (sizeof(plDrawState));
  
  /* copy from old state unless there wasn't any (i.e. an empty stack), in
     which case copy from the default drawing state (see g_defstate.c) */
  copyfrom = oldstate ? oldstate : &_default_drawstate;
  memcpy (_plotter->drawstate, copyfrom, sizeof(plDrawState));

  /* elements of state that are strings are treated specially */
  fill_rule = (char *)_plot_xmalloc (strlen (copyfrom->fill_rule) + 1);
  line_mode = (char *)_plot_xmalloc (strlen (copyfrom->line_mode) + 1);
  join_mode = (char *)_plot_xmalloc (strlen (copyfrom->join_mode) + 1);
  cap_mode = (char *)_plot_xmalloc (strlen (copyfrom->cap_mode) + 1);
  strcpy (fill_rule, copyfrom->fill_rule);
  strcpy (line_mode, copyfrom->line_mode);
  strcpy (join_mode, copyfrom->join_mode);
  strcpy (cap_mode, copyfrom->cap_mode);
  _plotter->drawstate->fill_rule = fill_rule;
  _plotter->drawstate->line_mode = line_mode;
  _plotter->drawstate->join_mode = join_mode;
  _plotter->drawstate->cap_mode = cap_mode;

  /* dash array, if non-empty, is treated specially too */
  if (copyfrom->dash_array_len > 0)
    {
      int i;
      double *dash_array;

      dash_array = (double *)_plot_xmalloc (copyfrom->dash_array_len * sizeof(double));
      for (i = 0; i < copyfrom->dash_array_len; i++)
	dash_array[i] = copyfrom->dash_array[i];
      _plotter->drawstate->dash_array = dash_array;
    }

  /* The font_name, font_type, typeface_index, and font_index fields are
     special, since for the initial drawing state they're Plotter-dependent.

     For later drawing states, we just copy them from the previous state.
     Since only the first (font_name) is a string, for later states we
     don't worry about the other three: they've already been copied.

     The fill_rule_type field is also treated specially in the initial
     drawing state, because not all Plotters support both types of filling
     (odd vs. nonzero winding number). */

  if (oldstate)			/* a later state */
    {
      font_name = (char *)_plot_xmalloc (strlen (copyfrom->font_name) + 1);  
      strcpy (font_name, copyfrom->font_name);  
      _plotter->drawstate->font_name = font_name;
    }
  else				/* initial state, i.e. first on stack */
    {
      const char *font_name_init;
      int typeface_index, font_index;
      
      switch (_plotter->default_font_type)
	{
	case F_HERSHEY:
	default:
	  font_name_init = DEFAULT_HERSHEY_FONT;
	  typeface_index = DEFAULT_HERSHEY_TYPEFACE_INDEX;
	  font_index = DEFAULT_HERSHEY_FONT_INDEX;	  
	  break;
	case F_POSTSCRIPT:
	  font_name_init = DEFAULT_POSTSCRIPT_FONT;
	  typeface_index = DEFAULT_POSTSCRIPT_TYPEFACE_INDEX;
	  font_index = DEFAULT_POSTSCRIPT_FONT_INDEX;	  
	  break;
	case F_PCL:
	  font_name_init = DEFAULT_PCL_FONT;
	  typeface_index = DEFAULT_PCL_TYPEFACE_INDEX;
	  font_index = DEFAULT_PCL_FONT_INDEX;	  
	  break;
	case F_STICK:
	  font_name_init = DEFAULT_STICK_FONT;
	  typeface_index = DEFAULT_STICK_TYPEFACE_INDEX;
	  font_index = DEFAULT_STICK_FONT_INDEX;	  
	  break;
	}

      font_name = (char *)_plot_xmalloc (strlen (font_name_init) + 1);
      strcpy (font_name, font_name_init);  
      _plotter->drawstate->font_name = font_name;
      _plotter->drawstate->font_type = _plotter->default_font_type;      
      _plotter->drawstate->typeface_index = typeface_index;
      _plotter->drawstate->font_index = font_index;      

      /* Examine default fill mode.  If Plotter doesn't support it, use the
	 other fill mode. */
      if (_plotter->drawstate->fill_rule_type == FILL_ODD_WINDING
	  && _plotter->have_odd_winding_fill == 0)
	_plotter->drawstate->fill_rule_type = FILL_NONZERO_WINDING;
      else if (_plotter->drawstate->fill_rule_type == FILL_NONZERO_WINDING
	  && _plotter->have_nonzero_winding_fill == 0)
	_plotter->drawstate->fill_rule_type = FILL_ODD_WINDING;
    }

  /* Our memcpy copied the pointer to the path under constrution (if any).
     So we knock it out, to start afresh */
  _plotter->drawstate->datapoints = NULL;
  _plotter->drawstate->points_in_path = 0;
  _plotter->drawstate->datapoints_len = 0;

  /* install new state at head of the state list */
  _plotter->drawstate->previous = oldstate;

  return 0;
}
