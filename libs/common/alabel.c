/* This file contains the alabel routine, which is a GNU extension to
   libplot.  It draws a label, i.e. a text string, at the current location.
   Horizontal and vertical justification must be specified.

   The alabel routine returns the width of the string in user units.  The
   return value is zero if no information on font size is available. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* ALABEL takes three arguments X_JUSTIFY, Y_JUSTIFY, and S, and places the
   label S according to the x and y axis adjustments specified in X_JUSTIFY
   and Y_JUSTIFY.  X_JUSTIFY is equal to 'l', 'c', or 'r', signifying
   left-justified, centered, or right-justified, relative to the current
   position.  Y_JUSTIFY is equal to 'b', 'x', 'c', or 't', signifying that
   the bottom, baseline, center, or top of the label should pass through
   the current position. */

double
falabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     char *s;
{
  int width = 0.0;
  char *t;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: falabel() called when graphics device not open\n");
      return -1;
    }

  endpoly ();			/* flush polyline if any */

  if (s == NULL)
    return 0.0;			/* avoid core dumps */

  /* copy because we may alter the string */
  t = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  if (_drawstate->font_type == F_STANDARD
      || _drawstate->font_type == F_STROKE)
    /* allow only character set in ISO encoding */
    {
      Boolean was_clean;
      
      was_clean = _clean_iso_string ((unsigned char *)t);
      if (!was_clean)
	fprintf (stderr, 
		 "libplot: ignoring control character (e.g. CR or LF) in label\n");
    }
  
  if (*t)
    {
      unsigned short *codestring;

      switch (_drawstate->font_type)
	{
	case F_STANDARD:
	  codestring = _controlify ((unsigned char *)t);
	  width = _alabel_standard (x_justify, y_justify, codestring);
	  free (codestring);
	  break;
	  
	case F_STROKE:
	  codestring = _controlify ((unsigned char *)t);
	  width = _alabel_stroke (x_justify, y_justify, codestring);
	  free (codestring);
	  break;
	  
	case F_DEVICE_SPECIFIC:
	  codestring = _controlify ((unsigned char *)t);
	  width = _alabel_device (x_justify, y_justify, codestring);
	  free (codestring);
	  break;
	  
	default:
	  width = 0.0;
	  break;
	}
    }
  
  free (t);
  return width;
}

int
alabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     char *s;
{
  double width = falabel (x_justify, y_justify, s);
  
  return IROUND(width);
}
