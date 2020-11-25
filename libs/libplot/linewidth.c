/* This file contains the linewidth routine, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
linewidth (new_line_width)
     int new_line_width;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: linewidth() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %d\n", 
	     LINEWIDTH, new_line_width);
  else
    {
      putc (LINEWIDTH, _outstream);
      _putshort (new_line_width, _outstream);
    }
  
  return 0;
}

int
flinewidth (new_line_width)
     double new_line_width;
{
  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: flinewidth() called when graphics device not open\n");
      return -1;
    }

  if (_libplot_output_is_ascii)
    fprintf (_outstream, "%c %g\n", 
	     FLINEWIDTH, new_line_width);
  else
    {
      putc (FLINEWIDTH, _outstream);
      _putfloat (new_line_width, _outstream);
    }
  
  return 0;
}
