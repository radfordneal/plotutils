/* This file contains the labelwidth routine, which is a GNU extension to
   libplot.  It returns the width, in user units, of a label, i.e., a text
   string.  A return value of zero may mean that no information on font
   size is available. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
flabelwidth (s)
     char *s;
{
  double width = 0.0;
  char *t;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: labelwidth() called when graphics device not open\n");
      return -1;
    }

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
	  width = _labelwidth_standard (codestring);
	  free (codestring);
	  break;
	  
	case F_STROKE:
	  codestring = _controlify ((unsigned char *)t);
	  width = _labelwidth_stroke (codestring);
	  free (codestring);
	  break;
	  
	case F_DEVICE_SPECIFIC:
	  codestring = _controlify ((unsigned char *)t);
	  width = _labelwidth_device (codestring);
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
labelwidth (s)
     char *s;
{
  double width = flabelwidth (s);
  
  return IROUND(width);
}
