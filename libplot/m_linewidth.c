/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linewidth (int new_line_width)
#else
_m_linewidth (new_line_width)
     int new_line_width;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("linewidth: invalid operation");
      return -1;
    }

  /* invoke generic method */
  _g_flinewidth ((double)new_line_width);

  _meta_emit_byte ((int)O_LINEWIDTH);
  _meta_emit_integer (new_line_width);
  _meta_emit_terminator ();
  
  return 0;
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

  /* invoke generic method */
  _g_flinewidth (new_line_width);

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_LINEWIDTH : (int)O_FLINEWIDTH);
  _meta_emit_float (new_line_width);
  _meta_emit_terminator ();
  
  return 0;
}
