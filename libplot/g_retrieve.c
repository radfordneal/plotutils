/* This file contains the internal _retrieve_font method, which is called
   when when the font_name, font_size, and textangle fields of the current
   drawing state have been filled in.  It retrieves the specified font, and
   fills in the font_type, typeface_index, font_index, font_is_iso8858_1,
   true_font_size, font_ascent, and font_descent fields of the drawing
   state. */

/* This is a generic method.  Some types of Plotter may do something else.
   E.g., a Fig Plotter invokes this, and if the retrieved font is a
   Postscript font, quantizes the size (see f_retrieve.c).  X and X
   Drawable Plotters have their own retrieval method and don't invoke this
   at all (see x_retrieve.c). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_her_metr.h"

static bool _match_ps_font __P ((void));
static bool _match_pcl_font __P ((void));

void
#ifdef _HAVE_PROTOS
_g_retrieve_font(void)
#else
_g_retrieve_font()
#endif
{
  char *old_font_name;
  int i;

  /* try to match user-specified font name */

  if (_plotter->have_hershey_fonts)
    /* should always be true... */
    {
      /* is font a vector font? */
      i = -1;
      while (_vector_font_info[++i].name)
	{
	  if (_vector_font_info[i].visible) /* i.e. font not internal-only */
	    if (strcasecmp (_vector_font_info[i].name, 
			    _plotter->drawstate->font_name) == 0
		|| (_vector_font_info[i].othername
		    && strcasecmp (_vector_font_info[i].othername, 
				   _plotter->drawstate->font_name) == 0))
	      {
		_plotter->drawstate->font_type = F_HERSHEY;
		_plotter->drawstate->typeface_index = 
		  _vector_font_info[i].typeface_index;
		_plotter->drawstate->font_index = 
		  _vector_font_info[i].font_index;
		_plotter->drawstate->font_is_iso8859_1 = 
		  _vector_font_info[i].iso8859_1;
		_plotter->drawstate->true_font_size = 
		  _plotter->drawstate->font_size;
		/* N.B. this macro uses true_font_size, see g_alabel_her.c */
		_plotter->drawstate->font_ascent = 
		  HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_ASCENT);
		_plotter->drawstate->font_descent = 
		  HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_DESCENT);
		return;
	      }
	} 
    }

  /* Try to match the font name with the name of a PCL font or a PS font.
     However there is a namespace collision: "Courier" etc. and "Symbol"
     occur on both lists.  So which list we search first depends on what
     type of Plotter we have. */

  switch (_plotter->type)
    {
    case PL_PS:
    default:
      /* search PS font list first */
      if (_plotter->have_ps_fonts && _match_ps_font())
	return;
      if (_plotter->have_pcl_fonts && _match_pcl_font())
	return;
      break;
    case PL_HPGL:
    case PL_PCL:
      /* search PCL font list first */      
      if (_plotter->have_pcl_fonts && _match_pcl_font())
	return;
      if (_plotter->have_ps_fonts && _match_ps_font())
	return;
      break;
    }

  if (_plotter->have_stick_fonts)
    {
      /* is font a Stick font? */
      i = -1;
      while (_stick_font_info[++i].ps_name)
	{
	  if (_stick_font_info[i].basic == false
	      && !_plotter->have_extra_stick_fonts)
	    /* not a basic Stick font, and this Plotter supports only the
               basic ones */
	    continue;

	  if (strcasecmp (_stick_font_info[i].ps_name, 
			  _plotter->drawstate->font_name) == 0)
	    {
	      _plotter->drawstate->font_type = F_STICK;
	      _plotter->drawstate->typeface_index = 
		_stick_font_info[i].typeface_index;
	      _plotter->drawstate->font_index = 
		_stick_font_info[i].font_index;
	      _plotter->drawstate->font_is_iso8859_1 = 
		_stick_font_info[i].iso8859_1;
	      _plotter->drawstate->true_font_size = 
		_plotter->drawstate->font_size;
	      _plotter->drawstate->font_ascent 
		= _plotter->drawstate->true_font_size
		  * (double)(_stick_font_info[i].font_ascent)/1000.0;
	      _plotter->drawstate->font_descent 
		= _plotter->drawstate->true_font_size
		  * (double)(_stick_font_info[i].font_descent)/1000.0;
	      return;
	    }
	}
    }

  /* Squawk if a substitution will be made, unless this is a Metafile
     Plotter (in which case we'll substitute silently, and hope that string
     widths won't be too far off). */
  if (_plotter->type != PL_META && !_plotter->font_warning_issued)
    {
      char *buf;
      
      buf = (char *)_plot_xmalloc (strlen (_plotter->drawstate->font_name) + strlen (_plotter->default_drawstate->font_name) + 100);
      sprintf (buf, "cannot retrieve font \"%s\", using default \"%s\"", 
	       _plotter->drawstate->font_name, 
	       _plotter->default_drawstate->font_name);
      _plotter->warning (buf);
      free (buf);
      _plotter->font_warning_issued = true;
    }

  old_font_name = _plotter->drawstate->font_name;
  _plotter->drawstate->font_name = _plotter->default_drawstate->font_name;
  /* do recursive call */
  _g_retrieve_font();
  _plotter->drawstate->font_name = old_font_name;
  
  return;
}

static bool
#ifdef _HAVE_PROTOS
_match_pcl_font (void)
#else
_match_pcl_font ()
#endif
{
  int i = -1;

  /* is font a PCL font? */
  while (_pcl_font_info[++i].ps_name)
    {
      if (strcasecmp (_pcl_font_info[i].ps_name, 
		      _plotter->drawstate->font_name) == 0)
	{
	  _plotter->drawstate->font_type = F_PCL;
	  _plotter->drawstate->typeface_index = 
	    _pcl_font_info[i].typeface_index;
	  _plotter->drawstate->font_index = 
	    _pcl_font_info[i].font_index;
	  _plotter->drawstate->font_is_iso8859_1 = 
	    _pcl_font_info[i].iso8859_1;
	  _plotter->drawstate->true_font_size = 
	    _plotter->drawstate->font_size;
	  _plotter->drawstate->font_ascent 
	    = _plotter->drawstate->true_font_size
	      * (double)(_pcl_font_info[i].font_ascent)/1000.0;
	  _plotter->drawstate->font_descent 
	    = _plotter->drawstate->true_font_size
	      * (double)(_pcl_font_info[i].font_descent)/1000.0;
	  return true;
	}
    }
  
  return false;
}


static bool
#ifdef _HAVE_PROTOS
_match_ps_font (void)
#else
_match_ps_font ()
#endif
{
  int i = -1;

  /* is font a PS font? */
  while (_ps_font_info[++i].ps_name)
    {
      if (strcasecmp (_ps_font_info[i].ps_name, 
		      _plotter->drawstate->font_name) == 0
	  /* try alternative PS font name if any */
	  || (_ps_font_info[i].ps_name_alt != NULL
	      && strcasecmp (_ps_font_info[i].ps_name_alt, 
			     _plotter->drawstate->font_name) == 0)
	  || strcasecmp (_ps_font_info[i].x_name, 
			 _plotter->drawstate->font_name) == 0
	  /* try alternative X font name if any */
	  || (_ps_font_info[i].x_name_alt != NULL
	      && strcasecmp (_ps_font_info[i].x_name_alt,
			     _plotter->drawstate->font_name) == 0))
	{
	  _plotter->drawstate->font_type = F_POSTSCRIPT;
	  _plotter->drawstate->typeface_index = 
	    _ps_font_info[i].typeface_index;
	  _plotter->drawstate->font_index = 
	    _ps_font_info[i].font_index;
	  _plotter->drawstate->font_is_iso8859_1 = 
	    _ps_font_info[i].iso8859_1;
	  _plotter->drawstate->true_font_size = 
	    _plotter->drawstate->font_size;
	  _plotter->drawstate->font_ascent 
	    = _plotter->drawstate->true_font_size
	      * (double)(_ps_font_info[i].font_ascent)/1000.0;
	  _plotter->drawstate->font_descent 
	    = _plotter->drawstate->true_font_size
	      * (double)(_ps_font_info[i].font_descent)/1000.0;
	  return true;
	}
    }
  
  return false;
}
