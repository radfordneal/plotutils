/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linewidth (R___(Plotter *_plotter) int new_line_width)
#else
_m_linewidth (R___(_plotter) new_line_width)
     S___(Plotter *_plotter;) 
     int new_line_width;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "linewidth: invalid operation");
      return -1;
    }

  /* invoke generic method */
  _g_flinewidth (R___(_plotter) (double)new_line_width);

  _meta_emit_byte (R___(_plotter) (int)O_LINEWIDTH);
  _meta_emit_integer (R___(_plotter) new_line_width);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}

int
#ifdef _HAVE_PROTOS
_m_flinewidth (R___(Plotter *_plotter) double new_line_width)
#else
_m_flinewidth (R___(_plotter) new_line_width)
     S___(Plotter *_plotter;) 
     double new_line_width;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "flinewidth: invalid operation");
      return -1;
    }

  /* invoke generic method */
  _g_flinewidth (R___(_plotter) new_line_width);

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_LINEWIDTH : (int)O_FLINEWIDTH);
  _meta_emit_float (R___(_plotter) new_line_width);
  _meta_emit_terminator (S___(_plotter));
  
  return 0;
}
