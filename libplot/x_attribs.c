/* This internal method is invoked by an XDrawablePlotter (or XPlotter)
   before drawing any polyline.  It sets the relevant attributes in our X
   graphics contexts (line type, cap type, join type, line width, fill
   rule) to what they should be. */

#include "sys-defines.h"
#include "extern.h"

/* The length of each dash must fit in an unsigned char (X11 convention) */
#define MAX_DASH_LENGTH 255

void
#ifdef _HAVE_PROTOS
_x_set_attributes (void)
#else
_x_set_attributes ()
#endif
{
  XGCValues gcv;

  if (_plotter->drawstate->dash_array_in_effect)
    /* have user-specified dash array */
    {
      int num_dashes, offset;
      unsigned char *dashbuf;

      num_dashes = _plotter->drawstate->dash_array_len;
      if (num_dashes > 0)
	{
	  bool odd_length;
	  double min_sing_val, max_sing_val;
	  int i, dash_cycle_length;

	  /* compute minimum singular value of user->device coordinate map,
	     which we use as a multiplicative factor to convert line widths
	     (cf. g_linewidth.c), dash lengths, etc. */
	  _matrix_sing_vals (_plotter->drawstate->transform.m, 
			     &min_sing_val, &max_sing_val);

	  odd_length = (num_dashes & 1 ? true : false);
	  dashbuf = (unsigned char *)_plot_xmalloc ((odd_length ? 2 : 1) * num_dashes * sizeof(unsigned char));
	  dash_cycle_length = 0;
	  for (i = 0; i < num_dashes; i++)
	    {
	      double unrounded_dashlen;
	      int dashlen;

	      unrounded_dashlen = 
		min_sing_val * _plotter->drawstate->dash_array[i];

	      dashlen = IROUND(unrounded_dashlen);
	      dashlen = IMAX(dashlen, 1);
	      dashlen = IMIN(dashlen, MAX_DASH_LENGTH);

	      /* convert dash length, int -> unsigned char */
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

	  XSetDashes (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		      offset, (char *)dashbuf, num_dashes);
	  free (dashbuf);
	  gcv.line_style = LineOnOffDash;
	}
      else			/* no dashes, will draw as solid line */
	gcv.line_style = LineSolid;

    }
  else
    /* have one of the canonical line types */
    {
      if (_plotter->drawstate->line_type != L_SOLID)
	{
	  const int *dash_array;
	  unsigned char dashbuf[MAX_DASH_ARRAY_LEN];
	  int i, scale, num_dashes;

	  num_dashes = _line_styles[_plotter->drawstate->line_type].dash_array_len;	  
	  dash_array = _line_styles[_plotter->drawstate->line_type].dash_array;
	  /* scale by line width in terms of pixels, if nonzero */
	  scale = _plotter->drawstate->quantized_device_line_width;
	  if (scale <= 0)
	    scale = 1;
	  for (i = 0; i < num_dashes; i++)
	    {
	      int dashlen;
	      
	      dashlen = scale * dash_array[i];
	      dashlen = IMAX(dashlen, 1);
	      dashlen = IMIN(dashlen, MAX_DASH_LENGTH);
	      dashbuf[i] = (unsigned int)dashlen; /* int->unsigned char */
	    }
	  XSetDashes (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		      0, (char *)dashbuf, num_dashes);
	  gcv.line_style = LineOnOffDash;	  
	}
      else			/* no dashes */
	gcv.line_style = LineSolid;
    }
  
  /* in GC, set line cap type */
  switch (_plotter->drawstate->cap_type)
    {
    case CAP_BUTT:
    default:
      gcv.cap_style = CapButt;
      break;
    case CAP_ROUND:
      gcv.cap_style = CapRound;
      break;
    case CAP_PROJECT:
      gcv.cap_style = CapProjecting;
      break;
    case CAP_TRIANGULAR:	/* not supported by X11 */
      gcv.cap_style = CapRound;
      break;
    }

  /* in GC, set line join type */
  switch (_plotter->drawstate->join_type)
    {
    case JOIN_MITER:
    default:
      gcv.join_style = JoinMiter;
      break;
    case JOIN_ROUND:
      gcv.join_style = JoinRound;
      break;
    case JOIN_BEVEL:
      gcv.join_style = JoinBevel;
      break;
    case JOIN_TRIANGULAR:	/* not supported by X11 */
      gcv.join_style = JoinRound;
      break;
    }

  /* in GC, set fill rule */
  switch (_plotter->drawstate->fill_rule_type)
    {
    case FILL_ODD_WINDING:
    default:
      gcv.fill_rule = EvenOddRule;
      break;
    case FILL_NONZERO_WINDING:
      gcv.fill_rule = WindingRule;
      break;
    }

  /* in GC, set line width in device coors (pixels) */
  gcv.line_width = _plotter->drawstate->quantized_device_line_width;

  /* make the same changes to both our GC's: the one used for drawing, and
     the one used for filling (this is overdoing things a bit) */
  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
	     GCLineStyle|GCCapStyle|GCJoinStyle|GCLineWidth|GCFillRule, &gcv);
  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill, 
	     GCLineStyle|GCCapStyle|GCJoinStyle|GCLineWidth|GCFillRule, &gcv);
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events();

  return;
}
