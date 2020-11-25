/* This file contains a function called by PNM and GIF Plotters, to fill in
   many of the fields in the graphics context used by the MI scan
   conversion routines, just before drawing. */

/* This calls malloc to allocate storage for the dash array in the graphics
   context, so free() will need to be invoked later 
   (on ((miGC*)ptr)->dash). */

#include "sys-defines.h"
#include "extern.h"
#include "g_mi.h"		/* use MI scan conversion module */

/* MI join styles, indexed by internal number (miter/rd./bevel/triangular) */
static const int _mi_join_style[] =
{ miJoinMiter, miJoinRound, miJoinBevel, miJoinTriangular };

/* MI cap styles, indexed by internal number (butt/rd./project/triangular) */
static const int _mi_cap_style[] =
{ miCapButt, miCapRound, miCapProjecting, miCapTriangular };

void
#ifdef _HAVE_PROTOS
_set_common_mi_attributes (Voidptr ptr)
#else
_set_common_mi_attributes (ptr)
     Voidptr ptr;
#endif
{
  int line_style, num_dashes, offset;
  unsigned int *dashbuf;
  miGC *miDataPtr;

  miDataPtr = (miGC *)ptr;

  miDataPtr->fillStyle = miFillSolid;
  miDataPtr->fillRule = 
    (_plotter->drawstate->fill_rule_type == FILL_NONZERO_WINDING ? 
     miWindingRule : miEvenOddRule);
  miDataPtr->joinStyle = _mi_join_style[_plotter->drawstate->join_type];
  miDataPtr->capStyle = _mi_cap_style[_plotter->drawstate->cap_type];
  miDataPtr->lineWidth = _plotter->drawstate->quantized_device_line_width;
  miDataPtr->miterLimit = _plotter->drawstate->miter_limit;
  miDataPtr->arcMode = miArcChord;	/* libplot convention */

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
	  
	  line_style = miLineOnOffDash;
	  odd_length = (num_dashes & 1 ? true : false);
	  dashbuf = (unsigned int *)_plot_xmalloc ((odd_length ? 2 : 1) * num_dashes * sizeof(unsigned int));
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
	  line_style = miLineSolid;
	  dashbuf = NULL;
	  offset = 0;
	}
    }
  else
    /* have one of the canonical line types */
    {
      if (_plotter->drawstate->line_type == L_SOLID)
	{
	  line_style = miLineSolid;
	  num_dashes = 0;
	  dashbuf = NULL;
	  offset = 0;
	}
      else
	{
	  const int *dash_array;
	  int scale, i;
	  
	  line_style = miLineOnOffDash;
	  num_dashes =
	    _line_styles[_plotter->drawstate->line_type].dash_array_len;
	  dash_array = _line_styles[_plotter->drawstate->line_type].dash_array;
	  dashbuf = (unsigned int *)_plot_xmalloc (MAX_DASH_ARRAY_LEN * sizeof(unsigned int));
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
	      dashbuf[i] = (unsigned int)dashlen; /* int->unsigned char */
	    }
	}
    }

  /* set dash-related fields in MI graphics context */
  miDataPtr->lineStyle = line_style;
  miDataPtr->numInDashList = num_dashes;
  miDataPtr->dash = dashbuf;
  miDataPtr->dashOffset = offset;
}
