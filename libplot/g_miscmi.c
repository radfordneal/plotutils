/* This file contains a function called by bitmap Plotters such as PNM and
   GIF Plotters, just before drawing.  It sets the attributes in the
   graphics context (of type `miGC') used by the libxmi scan conversion
   routines. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"		/* use libxmi scan conversion module */

/* libxmi joinstyles, indexed by internal number (miter/rd./bevel/triangular)*/
static const int _mi_join_style[] =
{ MI_JOIN_MITER, MI_JOIN_ROUND, MI_JOIN_BEVEL, MI_JOIN_TRIANGULAR };

/* libxmi capstyles, indexed by internal number (butt/rd./project/triangular)*/
static const int _mi_cap_style[] =
{ MI_CAP_BUTT, MI_CAP_ROUND, MI_CAP_PROJECTING, MI_CAP_TRIANGULAR };

void
#ifdef _HAVE_PROTOS
_set_common_mi_attributes (R___(Plotter *_plotter) voidptr_t ptr)
#else
_set_common_mi_attributes (R___(_plotter) ptr)
     S___(Plotter *_plotter;) 
     voidptr_t ptr;		/* really an (miGC *) */
#endif
{
  int line_style, num_dashes, offset;
  unsigned int *dashbuf;
  miGC *pGC;
  bool dash_array_allocated = false;
  miGCAttribute attributes[5];
  int values [5];
  unsigned int local_dashbuf[MAX_DASH_ARRAY_LEN];

  pGC = (miGC *)ptr;		/* convert to (miGC *) */

  /* set all miGC attributes that are not dash related */

  /* set five integer-valued miGC attributes */
  attributes[0] = MI_GC_FILL_RULE;
  values[0] = (_plotter->drawstate->fill_rule_type == FILL_NONZERO_WINDING ? 
	       MI_WINDING_RULE : MI_EVEN_ODD_RULE);
  attributes[1] = MI_GC_JOIN_STYLE;
  values[1] = _mi_join_style[_plotter->drawstate->join_type];
  attributes[2] = MI_GC_CAP_STYLE;
  values[2] = _mi_cap_style[_plotter->drawstate->cap_type];
  attributes[3] = MI_GC_ARC_MODE;
  values[3] = MI_ARC_CHORD;	/* libplot convention */
  attributes[4] = MI_GC_LINE_WIDTH;
  values[4] = _plotter->drawstate->quantized_device_line_width;
  miSetGCAttribs (pGC, 5, attributes, values);

  /* set a double-valued miGC attribute */
  miSetGCMiterLimit (pGC, _plotter->drawstate->miter_limit);

  /* now determine and set dashing-related attributes */

  if (_plotter->drawstate->dash_array_in_effect)
    /* have user-specified dash array */
    {
      int i;
      
      num_dashes = _plotter->drawstate->dash_array_len;
      if (num_dashes > 0)
	/* non-solid line type */
	{
	  bool odd_length;
	  double min_sing_val, max_sing_val;
	  int dash_cycle_length;

	  /* compute minimum singular value of user->device coordinate map,
	     which we use as a multiplicative factor to convert line widths
	     (cf. g_linewidth.c), dash lengths, etc. */
	  _matrix_sing_vals (_plotter->drawstate->transform.m, 
			     &min_sing_val, &max_sing_val);
	  
	  line_style = MI_LINE_ON_OFF_DASH;
	  odd_length = (num_dashes & 1 ? true : false);
	  {
	    int array_len;
	    
	    array_len = (odd_length ? 2 : 1) * num_dashes;
	    if (array_len <= MAX_DASH_ARRAY_LEN)
	      dashbuf = local_dashbuf; /* use dash buffer on stack */
	    else
	      {
		dashbuf = (unsigned int *)_plot_xmalloc (array_len * sizeof(unsigned int));
		dash_array_allocated = true;
	      }
	  }
	  dash_cycle_length = 0;
	  for (i = 0; i < num_dashes; i++)
	    {
	      double unrounded_dashlen;
	      int dashlen;

	      unrounded_dashlen = 
		min_sing_val * _plotter->drawstate->dash_array[i];

	      dashlen = IROUND(unrounded_dashlen);
	      dashlen = IMAX(dashlen, 1);

	      dashbuf[i] = (unsigned int)dashlen;
	      dash_cycle_length += dashlen;
	      if (odd_length)
		{
		  dashbuf[num_dashes + i] = (unsigned int)dashlen;
		  dash_cycle_length += dashlen;
		}
	    }
	  if (odd_length)
	    num_dashes *= 2;

	  offset = IROUND(min_sing_val * _plotter->drawstate->dash_offset);
	  if (dash_cycle_length > 0)
	    /* choose an offset in range 0..dash_cycle_length-1 */
	    {
	      while (offset < 0)
		offset += dash_cycle_length;
	      offset %= dash_cycle_length;
	    }
	}
      else
	/* zero-length dash array, i.e. solid line type */
	{
	  line_style = MI_LINE_SOLID;
	  dashbuf = NULL;
	  offset = 0;
	}
    }
  else
    /* have one of the canonical line types */
    {
      if (_plotter->drawstate->line_type == L_SOLID)
	{
	  line_style = MI_LINE_SOLID;
	  num_dashes = 0;
	  dashbuf = NULL;
	  offset = 0;
	}
      else
	{
	  const int *dash_array;
	  int scale, i;
	  
	  line_style = MI_LINE_ON_OFF_DASH;
	  num_dashes =
	    _line_styles[_plotter->drawstate->line_type].dash_array_len;
	  dash_array = _line_styles[_plotter->drawstate->line_type].dash_array;
	  dashbuf = local_dashbuf; /* it is large enough */
	  offset = 0;

	  /* scale by line width in terms of pixels, if nonzero */
	  scale = _plotter->drawstate->quantized_device_line_width;
	  if (scale <= 0)
	    scale = 1;

	  for (i = 0; i < num_dashes; i++)
	    {
	      int dashlen;
	      
	      dashlen = scale * dash_array[i];
	      dashlen = IMAX(dashlen, 1);
	      dashbuf[i] = (unsigned int)dashlen;
	    }
	}
    }

  /* set dash-related attributes in libxmi's graphics context */
  miSetGCAttrib (pGC, MI_GC_LINE_STYLE, line_style);
  if (line_style != (int)MI_LINE_SOLID)
    miSetGCDashes (pGC, num_dashes, dashbuf, offset);

  if (dash_array_allocated)
    free (dashbuf);
}
