/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes of an HP-GL or HP-GL/2 plotter (line type, cap
   type, join type, line width) to what they should be. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* HP-GL's line types, indexed into by internal line number
   (L_SOLID/L_DOTTED/ L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED. */
static const int _hpgl_line_type[] =
{ HPGL_L_SOLID, HPGL_L_DOTTED, HPGL_L_DOTDASHED, 
    HPGL_L_SHORTDASHED, HPGL_L_LONGDASHED };

/* HP-GL/2 join styles, indexed by internal join type number(miter/rd./bevel)*/
static const int _hpgl_join_style[] =
{ HPGL_JOIN_MITER, HPGL_JOIN_ROUND, HPGL_JOIN_BEVEL };

/* HP-GL/2 cap styles, indexed by internal cap type number (butt/rd./project)*/
static const int _hpgl_cap_style[] =
{ HPGL_CAP_BUTT, HPGL_CAP_ROUND, HPGL_CAP_PROJECT };

void
#ifdef _HAVE_PROTOS
_h_set_attributes (void)
#else
_h_set_attributes ()
#endif
{
  /* if plotter's line type doesn't agree with what it should be,
     adjust it */
  if (_plotter->hpgl_line_type != 
      _hpgl_line_type[_plotter->drawstate->line_type])
    {
      if (_hpgl_line_type[_plotter->drawstate->line_type] == HPGL_L_SOLID)
	strcpy (_plotter->outbuf.current, "LT;");
      else if (_hpgl_line_type[_plotter->drawstate->line_type] == HPGL_L_DOTTED)
	/* emulate dots by selecting shortdashed pattern along with a short
           iteration interval (0.5% of diagonal P1-P2 distance) */
	sprintf (_plotter->outbuf.current, 
		 "LT%d,0.5;", HPGL_L_SHORTDASHED);
      else
	/* iteration interval will be 2% of diagonal P1-P2 distance */
	sprintf (_plotter->outbuf.current, 
		 "LT%d,2;", _hpgl_line_type[_plotter->drawstate->line_type]);
      _update_buffer (&_plotter->outbuf);
      _plotter->hpgl_line_type = _hpgl_line_type[_plotter->drawstate->line_type];
    }

  /* if plotter's line attributes don't agree with what they should be,
     adjust them (HP-GL/2 only) */
  if (_plotter->hpgl_version == 2)
    {
      if ((_plotter->hpgl_cap_style 
	   != _hpgl_cap_style[_plotter->drawstate->cap_type])
	  || (_plotter->hpgl_join_style 
	      != _hpgl_join_style[_plotter->drawstate->join_type]))
	{
	  sprintf (_plotter->outbuf.current, "LA1,%d,2,%d;", 
		   _hpgl_cap_style[_plotter->drawstate->cap_type],
		   _hpgl_join_style[_plotter->drawstate->join_type]);
	  _update_buffer (&_plotter->outbuf);
	  _plotter->hpgl_cap_style = 
	    _hpgl_cap_style[_plotter->drawstate->cap_type];
	  _plotter->hpgl_join_style = 
	    _hpgl_join_style[_plotter->drawstate->join_type];
	}
    }
  
  /* if plotter's pen width doesn't agree with what it should be,
     update it (HP-GL/2 only) */
  if (_plotter->hpgl_version == 2)
    {
      if (_plotter->pen_width != _plotter->drawstate->hpgl_pen_width)
	{
	  sprintf (_plotter->outbuf.current, "PW%.3f;", 
		   100.0 * _plotter->drawstate->hpgl_pen_width);
	  _update_buffer (&_plotter->outbuf);
	  _plotter->pen_width = _plotter->drawstate->hpgl_pen_width;
	}
    }

  return;
}
