/* This file contains the fontname routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the name of the font used for
   text subsequent drawn on the graphics device.  The selected font may
   also be changed by calling the fontname routine.

   The fontname, fontsize, and textangle routines return the fontsize in
   user units, as an aid to vertical positioning by the user.  (The
   fontsize is normally taken to be an minimum vertical spacing between
   adjacent lines of text.)  A return value of zero means that no
   information on font size is available.

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence on the arguments given to the
   space() routine. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* FONTNAME takes a string argument S containing the name of the
   desired current font and sets the current font to that name.
   If S is a null pointer, the font is reset to the default. */

double
ffontname (s)
     char *s;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: ffontname() called when graphics device not open\n");
      return -1;
    }

  /* invalid pointer resets to default */
  if ((s == NULL) || (*s == '\0') || !strcmp(s, "(null)"))
    s = _default_drawstate.font_name;

  if (strcasecmp (_drawstate->font_name, s) != 0) /* font change */
    {
      int i;
      Boolean matched;

      /* save new font name */
      free (_drawstate->font_name);
      _drawstate->font_name = (char *)_plot_xmalloc (strlen (s) + 1);
      strcpy (_drawstate->font_name, s);

      matched = FALSE;

      /* is font a vector font? */
      i = -1;
      while (_vector_font_info[++i].name)
	{
	  if (_vector_font_info[i].visible) /* i.e. font not internal-only */
	    if (strcasecmp (_vector_font_info[i].name, _drawstate->font_name) == 0
		|| strcasecmp (_vector_font_info[i].othername, _drawstate->font_name) == 0)
	      {
		matched = TRUE;
		_drawstate->font_type = F_STROKE;
		_drawstate->typeface_index = _vector_font_info[i].typeface_index;
		_drawstate->font_index = _vector_font_info[i].font_index;
		_drawstate->font_is_iso8859 = _vector_font_info[i].iso8859_1;
		break;
	      }
	}

      if (!matched)
	{
	  /* is font a PS font? */
	  i = -1;
	  while (_ps_font_info[++i].ps_name)
	    {
	      if (strcasecmp (_ps_font_info[i].ps_name, _drawstate->font_name) == 0
		  || strcasecmp (_ps_font_info[i].x_name, _drawstate->font_name) == 0)
		{
		  matched = TRUE;
		  _drawstate->font_type = F_STANDARD;
		  _drawstate->typeface_index = _ps_font_info[i].typeface_index;
		  _drawstate->font_index = _ps_font_info[i].font_index;
		  _drawstate->font_is_iso8859 = _ps_font_info[i].iso8859_1;
		  break;
		}
	    }
	}
	  
      if (!matched)
	/* not a recognized font, so use default */
	{
	  fprintf (stderr, 
		   "libplot: unable to find font `%s', using default font `%s'\n", 
		   _drawstate->font_name, _default_drawstate.font_name);
	  
	  _drawstate->font_type = _default_drawstate.font_type;
	  _drawstate->typeface_index = _default_drawstate.typeface_index;
	  _drawstate->font_index = _default_drawstate.font_index;
	  _drawstate->font_is_iso8859 = _default_drawstate.font_is_iso8859;
	}
	  
      /* font change, so recompute fig_quantized_font_size,
         fig_font_point_size fields in drawing state (latter only if font
         is a PS font) */
      _set_font_sizes();
    }

  /* return closest integer to quantized version of user-specified font size */
  return _drawstate->fig_quantized_font_size;
}
