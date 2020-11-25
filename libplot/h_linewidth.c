/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_flinewidth(R___(Plotter *_plotter) double new_line_width)
#else
_h_flinewidth(R___(_plotter) new_line_width)
     S___(Plotter *_plotter;)
     double new_line_width;
#endif
{
  double width, height, diagonal_p1_p2_distance;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) "flinewidth: invalid operation");
      return -1;
    }

  /* invoke generic method */
  _g_flinewidth (R___(_plotter) new_line_width);

  /* set linewidth in scaled HP-GL coors also (i.e. as fraction of diagonal
     distance between P1,P2) */
  width = (double)(HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
  height = (double)(HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
  diagonal_p1_p2_distance = sqrt (width * width + height * height);
  _plotter->drawstate->hpgl_pen_width 
    = _plotter->drawstate->device_line_width / diagonal_p1_p2_distance;

  return 0;
}
