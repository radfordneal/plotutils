/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linewidth (int new_line_width)
#else
_m_linewidth (new_line_width)
     int new_line_width;
#endif
{
  double retval;

  if (!_plotter->open)
    {
      _plotter->error ("linewidth: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %d\n", 
		 LINEWIDTH, new_line_width);
      else
	{
	  putc (LINEWIDTH, _plotter->outstream);
	  _emit_integer (new_line_width);
	}
    }
  
  /* invoke generic method */
  retval = _g_flinewidth ((double)new_line_width);

  return IROUND(retval);
}

int
#ifdef _HAVE_PROTOS
_m_flinewidth (double new_line_width)
#else
_m_flinewidth (new_line_width)
     double new_line_width;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("flinewidth: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g\n", 
		 LINEWIDTH, new_line_width);
      else
	{
	  putc (FLINEWIDTH, _plotter->outstream);
	  _emit_float (new_line_width);
	}
    }
  
  /* invoke generic method */
  return _g_flinewidth (new_line_width);
}
