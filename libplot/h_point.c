/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* size of a `point' as fraction of diagonal distance between scaling
   points P1 and P2, i.e. as fraction of distance between opposite corners
   of the graphics display */
#define POINT_HPGL_SIZE 0.0001

int
#ifdef _HAVE_PROTOS
_h_fpoint (double x, double y)
#else
_h_fpoint (x, y)
     double x, y;
#endif
{
  double saved_width;
  int saved_join_type, saved_cap_type;

  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  /* select appropriate pen */
  _plotter->set_pen_color ();

  /* temporarily store pen width and line attributes */
  saved_width = _plotter->drawstate->hpgl_pen_width;
  saved_join_type = _plotter->drawstate->join_type;
  saved_cap_type = _plotter->drawstate->cap_type;  
  _plotter->drawstate->hpgl_pen_width = POINT_HPGL_SIZE;
  _plotter->drawstate->join_type = JOIN_ROUND;
  _plotter->drawstate->cap_type = CAP_ROUND;  

  /* sync pen position and line attributes, incl. pen width */
  _plotter->set_position();
  _plotter->set_attributes();

  if (_plotter->pendown == false)
    /* if pen were down, point would be invisible */
    {
      strcpy (_plotter->outbuf.current, "PD;");
      _update_buffer (&_plotter->outbuf);
      _plotter->pendown = true;
    }

  /* restore line attributes, incl. pen width */
  _plotter->drawstate->hpgl_pen_width = saved_width;
  _plotter->drawstate->join_type = saved_join_type;
  _plotter->drawstate->cap_type = saved_cap_type;

  return 0;
}
