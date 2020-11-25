/* This file contains the internal _retrieve_font method, which is called
   when when the font_name, font_size, and textangle fields of the current
   drawing state have been filled in.  It retrieves the specified font, and
   fills in the font_type, typeface_index, font_index, font_is_iso8858,
   true_font_size, and font_ascent, and font_descent fields of the drawing
   state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_her_metr.h"

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
    {
      /* is font a vector font? */
      i = -1;
      while (_vector_font_info[++i].name)
	{
	  if (_vector_font_info[i].visible) /* i.e. font not internal-only */
	    if (strcasecmp (_vector_font_info[i].name, 
			    _plotter->drawstate->font_name) == 0
		|| strcasecmp (_vector_font_info[i].othername, 
			       _plotter->drawstate->font_name) == 0)
	      {
		_plotter->drawstate->font_type = F_HERSHEY;
		_plotter->drawstate->typeface_index = 
		  _vector_font_info[i].typeface_index;
		_plotter->drawstate->font_index = 
		  _vector_font_info[i].font_index;
		_plotter->drawstate->font_is_iso8859 = 
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

  if (_plotter->have_ps_fonts)
    {
      /* is font a PS font? */
      i = -1;
      while (_ps_font_info[++i].ps_name)
	{
	  if (strcasecmp (_ps_font_info[i].ps_name, 
			  _plotter->drawstate->font_name) == 0
	      || strcasecmp (_ps_font_info[i].x_name, 
			     _plotter->drawstate->font_name) == 0)
	    {
	      _plotter->drawstate->font_type = F_POSTSCRIPT;
	      _plotter->drawstate->typeface_index = 
		_ps_font_info[i].typeface_index;
	      _plotter->drawstate->font_index = 
		_ps_font_info[i].font_index;
	      _plotter->drawstate->font_is_iso8859 = 
		_ps_font_info[i].iso8859_1;
	      _plotter->drawstate->font_ascent 
		= _plotter->drawstate->font_size 
		  * (double)(_ps_font_info[i].font_ascent)/1000.0;
	      _plotter->drawstate->font_descent 
		= _plotter->drawstate->font_size 
		  * (double)(_ps_font_info[i].font_descent)/1000.0;
	      _plotter->drawstate->true_font_size = 
		_plotter->drawstate->font_size;
	      return;
	    }
	}
    }
  
  if (_plotter->have_pcl_fonts)
    {
      /* is font a PCL font? */
      i = -1;
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
	      _plotter->drawstate->font_is_iso8859 = 
		(_pcl_font_info[i].pcl_symbol_set == PCL_ISO_8859_1);
	      _plotter->drawstate->font_ascent 
		= _plotter->drawstate->font_size 
		  * (double)(_pcl_font_info[i].font_ascent)/1000.0;
	      _plotter->drawstate->font_descent 
		= _plotter->drawstate->font_size 
		  * (double)(_pcl_font_info[i].font_descent)/1000.0;
	      _plotter->drawstate->true_font_size = 
		_plotter->drawstate->font_size;
	      return;
	    }
	}
    }

  if (!_plotter->font_warning_issued)
    {
      char *buf;
      
      buf = (char *)_plot_xmalloc (sizeof (_plotter->drawstate->font_name) + sizeof (_plotter->default_drawstate->font_name) + 100);
      sprintf (buf, "cannot retrieve font \"%s\", using default \"%s\"", 
	       _plotter->drawstate->font_name, 
	       _plotter->default_drawstate->font_name);
      _plotter->warning (buf);
      free (buf);
      _plotter->font_warning_issued = true;
    }

  old_font_name = _plotter->drawstate->font_name;
  _plotter->drawstate->font_name = _plotter->default_drawstate->font_name;
  _g_retrieve_font();
  _plotter->drawstate->font_name = old_font_name;
  
  return;
}
